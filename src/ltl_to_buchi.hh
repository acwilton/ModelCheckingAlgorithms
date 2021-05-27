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
      template <typename State, typename AP>
      struct LTLNode {
        static int count = 0;
        int id;

        auto_set<Formula<State, AP>> new;
        auto_set<Formula<State, AP>> now;
        auto_set<Formula<State, AP>> next;
      };

      template <typename State, AP>
      LTLNode freshNode(auto_set<Formula<State, AP>> new,
                        auto_set<Formula<State, AP>> now,
                        auto_set<Formula<State, AP>> next) {
        return LTLNode{++LTLNode::count, new, now, next};
      }

      struct NodeRelations {
        void add_relation(int fromNodeId, int toNodeId) {
          incoming[toNodeId].insert(fromNodeId);
          outgoing[fromNodeId].insert(toNodeId);
        }
        void remove_relation(int fromNodeId, int toNodeId) {
          incoming[toNodeId].erase(toNodeId);
          outgoing[fromNodeId].erase(toNodeId);
        }
        std::unordered_map<int, std::unordered_set<int>> incoming;
        std::unordered_map<int, std::unordered_set<int>> outgoing;
      };

      template <typename State, typename AP>
      void UpdateClosed(std::unordered_map<int, LTLNode<State, AP>>& open,
                        std::unordered_map<int, LTLNode<State, AP>>& closed,
                        NodeRelations& nodeRelations,
                        LTLNode<State, AP>& q) {
        bool foundMatch = false;
        for (auto& c : closed) {
          if (c.now == q.now && c.next == q.next) {
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
          closed[q.id] = q;
          auto nextNode = freshNode(q.next, {}, {});
          nodeRelations.add_relation(q.id, nextNode.id);
          open[nextNode.id] = nextNode;
        }
      }

      template <typename State, AP>
      void UpdateSplit(std::unordered_map<int, LTLNode<State, AP>>& open,
                       NodeRelations& nodeRelations,
                       auto_set<Formula<State,AP>>>& untilSet,
                       LTLNode<State, AP>& q,
                       Formula<State, AP>& psi) {
        auto Split = [&nodeRelations,&q]() {
          LTLNode<State,AP> splitQ = freshNode(q.new,q.now,q.next);
          for (int fromId : nodeRelations.incoming.at(q.id)) {
            nodeRelations.add_relation(fromId, splitQ.id);
          }
          return open.at(splitQ.id);
        }

        switch(psi.form()) {
        case FormulaForm::Atomic:
          break;

        case FormulaForm::Or:
          LTLNode<State, AP> splitQ = Split();
          q.new.insert(psi.getSubformulas()[0]);
          splitQ.new.insert(psi.getSubformulas()[1]);
          open[splitQ.id] = splitQ;
          break;

        case FormulaForm::And:
          q.new.insert(psi.getSubformulas()[0]);
          q.new.insert(psi.getSubformulas()[1]);
          break;

        case FormulaForm::Until:
          untilSet.insert(psi);
          LTLNode<State, AP> splitQ = Split();
          q.new.insert(psi.getSubformulas()[1]);
          splitQ.new.insert(psi.getSubformulas()[0]);
          splitQ.next.insert(psi);
          open[splitQ.id] = splitQ;
          break;

        case FormulaForm::Release:
          LTLNode<State, AP> splitQ = Split();
          q.new.insert(psi.getSubformulas()[0]);
          q.new.insert(psi.getSubformulas()[1]);
          splitQ.new.insert(psi.getSubformulas()[1]);
          splitQ.next.insert(psi);
          open[splitQ.id] = splitQ;
          break;

        default:
          std::string formString = (psi.form() == FormulaForm::Not) ? "Not"
            : ((psi.form() == FormulaForm::Global) ? "Global" : "Future");
          throw std::logic_error("FormulaForm encountered which should never appear in an LTLToBuchi conversion! Formula must be normalized first! Encountered FormulaForm: "+formString);
        }
      }

    }

    template <typename State, typename AP>
    auto LTLToBuchi(Formula<State, AP> const& formula) {
      using Formula = Formula<State, AP>;
      using Kripke = Kripke<LTLNode<State,AP>, AP>;
      using LTLNode = _details_::LTLNode<State,AP>;

      auto_set<Formula<State,AP>> untilSet(); // Used for generating the fairness characteristic functions

      std::unordered_map<int, LTLNode> closed();
      std::unordered_map<int, LTLNode> open();
      _details_::NodeRelations nodeRelations();

      auto firstNode = _details_::freshNode({formula}, {}{});
      open[firstNode.id] = firstNode;
      nodeRelations.add_relation(-1, LTLNode::count);

      while (!open.empty()) {
        LTLNode& q = *(open.end() - 1);
        if (q.new.empty()) {
          open.erase(open.end() - 1);
          _details_::UpdateClosed(open, closed, nodeRelations, q);
        } else {
          auto [psiIter,_] = q.now.insert(std::move(*(q.new.end() - 1)));
          q.new.erase(q.new.end() - 1);
          _details_::UpdateSplit(open, nodeRelations, q, *psiIter);
        }
      }

      // The set of initial states are precisely the nodes in closed that contain -1 in their incomingNodes set.
      auto_set<LTLNode> initStates;
      for (auto& startId : nodeRelations.incoming.at(-1)) {
        initStates.insert(closed.at(startId));
      }

      std::vector<Kripke::StateCharFunc> fairnessConstraints;
      for (auto& formula : untilSet) {
        fairnessConstraints.emplace_back([&formula](LTLNode const& q) {
          // formula is of the form (U a b). q satisfies the constraint if either q satisfies b or does not satisfy (U a b).
          return (q.now.count(formula.getSubformulas()[1]) == 1) || (q.now.count(formula) == 0);
        }));
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
            for (auto& sub : node.now) {
              if (sub.form() == FormulaForm::Atomic && sub.getAP() == ap) return true;
            }
            return false;
          }),
        formula.getAPSet());
    }
  }
}

#endif
