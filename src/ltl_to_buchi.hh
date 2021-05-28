#ifndef LTL_TO_BUCHI_HH
#define LTL_TO_BUCHI_HH

#include <unordered_map>
#include <unordered_set>
#include "auto_set.hh"
#include "auto_map.hh"

#include "ltl.hh"
#include "kripke.hh"
#include "kripke_to_buchi.hh"

namespace mc {
  namespace ltl {
    namespace _details_ {
      template <typename AP>
      struct LTLNode {
        inline static int count = 0;

        LTLNode() = default;
        LTLNode(LTLNode const&) = default;
        LTLNode(LTLNode&&) = default;
        LTLNode(int id,
                auto_set<Formula<AP>> const& newSet,
                auto_set<Formula<AP>> const& nowSet,
                auto_set<Formula<AP>> const& nextSet)
          : id(id),
            newSet(newSet),
            nowSet(nowSet),
            nextSet(nextSet)
          {}

        LTLNode& operator=(LTLNode const&) = default;
        LTLNode& operator=(LTLNode&&) = default;

        bool operator==(LTLNode const& rhs) const {
          return id == rhs.id
            && newSet == rhs.newSet
            && nowSet == rhs.nowSet
            && nextSet == rhs.nextSet;
        }
        bool operator!=(LTLNode const& rhs) const {
          return !(*this == rhs);
        }

        int id;
        auto_set<Formula<AP>> newSet;
        auto_set<Formula<AP>> nowSet;
        auto_set<Formula<AP>> nextSet;
      };

      template <typename AP>
      LTLNode<AP> freshNode(auto_set<Formula<AP>> const& newSet,
                            auto_set<Formula<AP>> const& nowSet,
                            auto_set<Formula<AP>> const& nextSet) {
        return LTLNode<AP>(++LTLNode<AP>::count, newSet, nowSet, nextSet);
      }

      struct NodeRelations {
        void add_relation(int fromNodeId, int toNodeId) {
          incoming[toNodeId].insert(fromNodeId);
          outgoing[fromNodeId].insert(toNodeId);
        }
        void remove_relation(int fromNodeId, int toNodeId) {
          incoming[toNodeId].erase(fromNodeId);
          outgoing[fromNodeId].erase(toNodeId);
        }
        std::unordered_map<int, std::unordered_set<int>> incoming;
        std::unordered_map<int, std::unordered_set<int>> outgoing;
      };

      template <typename AP>
      void UpdateClosed(std::unordered_map<int, LTLNode<AP>>& open,
                        std::unordered_map<int, LTLNode<AP>>& closed,
                        NodeRelations& nodeRelations,
                        LTLNode<AP> const& q) {
        bool foundMatch = false;
        for (auto& [_,c] : closed) {
          if (c.nowSet == q.nowSet && c.nextSet == q.nextSet) {
            foundMatch = true;
            auto qIncoming = nodeRelations.incoming[q.id];
            // Moving transitions a -> q to q -> c
            for (int incomingId : qIncoming) {
              nodeRelations.add_relation(incomingId, c.id);
              nodeRelations.remove_relation(incomingId, q.id);
            }
          }
        }
        if (!foundMatch) {
          closed.insert(std::make_pair(q.id,q));
          auto nextNode = freshNode(q.nextSet, {}, {});
          nodeRelations.add_relation(q.id, nextNode.id);
          open.insert(std::make_pair(nextNode.id,nextNode));
        }
      }

      template <typename AP>
      void UpdateSplit(std::unordered_map<int, LTLNode<AP>>& open,
                       NodeRelations& nodeRelations,
                       auto_set<Formula<AP>>& untilSet,
                       LTLNode<AP> & q,
                       Formula<AP> const& psi) {
        auto Split = [&nodeRelations,&q]() {
          LTLNode<AP> splitQ = freshNode(q.newSet,q.nowSet,q.nextSet);
          for (int fromId : nodeRelations.incoming.at(q.id)) {
            nodeRelations.add_relation(fromId, splitQ.id);
          }
          return splitQ;
        };

        switch(psi.form()) {
        case FormulaForm::Atomic: {
          break;
        }

        case FormulaForm::Or: {
          LTLNode<AP> splitQ = Split();
          q.newSet.insert(psi.getSubformulas()[0]);
          splitQ.newSet.insert(psi.getSubformulas()[1]);
          open.insert(std::make_pair(splitQ.id, splitQ));
          break;
        }

        case FormulaForm::And: {
          q.newSet.insert(psi.getSubformulas()[0]);
          q.newSet.insert(psi.getSubformulas()[1]);
          break;
        }

        case FormulaForm::Until: {
          untilSet.insert(psi);
          LTLNode<AP> splitQ = Split();
          q.newSet.insert(psi.getSubformulas()[1]);
          splitQ.newSet.insert(psi.getSubformulas()[0]);
          splitQ.nextSet.insert(psi);
          open.insert(std::make_pair(splitQ.id, splitQ));
          break;
        }

        case FormulaForm::Release: {
          LTLNode<AP> splitQ = Split();
          q.newSet.insert(psi.getSubformulas()[0]);
          q.newSet.insert(psi.getSubformulas()[1]);
          splitQ.newSet.insert(psi.getSubformulas()[1]);
          splitQ.nextSet.insert(psi);
          open.insert(std::make_pair(splitQ.id, splitQ));
          break;
        }

        default:
          std::string formString = (psi.form() == FormulaForm::Not) ? "Not"
            : ((psi.form() == FormulaForm::Global) ? "Global" : "Future");
          throw std::logic_error("FormulaForm encountered which should never appear in an LTLToBuchi conversion! Formula must be normalized first! Encountered FormulaForm: "+formString);
        }
      }

    }

    template <typename AP>
    auto LTLToBuchi(Formula<AP> const& formula) {
      using Formula = Formula<AP>;
      using LTLNode = _details_::LTLNode<AP>;
      using Kripke = Kripke<LTLNode, AP>;

      auto_set<Formula> untilSet{}; // Used for generating the fairness characteristic functions

      std::unordered_map<int, LTLNode> closed{};
      std::unordered_map<int, LTLNode> open{};
      _details_::NodeRelations nodeRelations{};

      auto firstNode = _details_::freshNode<AP>({formula}, {}, {});
      open.insert(std::make_pair(firstNode.id, firstNode));
      nodeRelations.add_relation(-1, firstNode.id);

      while (!open.empty()) {
        LTLNode& q = open.begin()->second;
        if (q.newSet.empty()) {
          open.erase(open.begin());
          _details_::UpdateClosed(open, closed, nodeRelations, q);
        } else {
          auto [psiIter,_] = q.nowSet.insert(std::move(*(q.newSet.begin())));
          q.newSet.erase(q.newSet.begin());
          _details_::UpdateSplit(open, nodeRelations, untilSet, q, *psiIter);
        }
      }

      // The set of initial states are precisely the nodes in closed that contain -1 in their incomingNodes set.
      auto_set<LTLNode> initStates;
      for (auto& startId : nodeRelations.outgoing.at(-1)) {
        initStates.insert(closed.at(startId));
      }

      std::vector<typename Kripke::StateCharFunc> fairnessConstraints;
      for (auto& formula : untilSet) {
        fairnessConstraints.emplace_back([&formula](LTLNode const& q) {
          // formula is of the form (U a b). q satisfies the constraint if either q satisfies b or does not satisfy (U a b).
          return (q.nowSet.count(formula.getSubformulas()[1]) == 1) || (q.nowSet.count(formula) == 0);
        });
      }

      return KripkeToBuchi(
        Kripke(
          initStates,
          [&closed,&nodeRelations](LTLNode const& node) {
            auto_set<LTLNode> nextSet;
            for (auto nextId : nodeRelations.outgoing.at(node.id)) {
              nextSet.insert(closed.at(nextId));
            }
            return nextSet;
          },
          fairnessConstraints,
          [](LTLNode const& node, AP const& ap) {
            for (auto& sub : node.nowSet) {
              if (sub.form() == FormulaForm::Atomic && sub.getAP() == ap) return true;
            }
            return false;
          }),
        formula.getAPSet());
    }
  }
}

#endif
