#ifndef BUCHI_UTILS_HH
#define BUCHI_UTILS_HH

#include <vector>
#include <optional>
#include <utility>
#include <any>

#include "buchi.hh"

namespace mc {
  // First component of Lasso is the sequence of elements up to, but not including the loop
  // Second component of Lasso is the loop of the lasso
  template <typename S>
  using Lasso = std::pair<std::vector<S>, std::vector<S>>;

  // Hiding implementation details under a namespace that is not meant to be accessed.
  namespace _details_ {
    template <typename S, typename A>
    std::optional<Lasso<S>> dfs2(Buchi<S,A> const& buchi, S const& q, std::vector<S> const& stack1, std::vector<S>& stack2, auto_set<S>& flagged) {
      flagged.insert(q);
      for (auto& [_,next] : buchi.getTransitions(q)) {
        auto iter = stack1.begin();
        for(; iter != stack1.end(); ++iter) {
          if (*iter == next) {
            std::vector<S> loop (iter, stack1.end());
            loop.insert(loop.end(), stack2.begin()+1, stack2.end());
            return std::make_optional(std::make_pair(std::vector<S>(stack1.begin(), iter), loop));
          }
        }
        if (flagged.count(next) == 0) {
          stack2.push_back(next);
          auto result = dfs2(buchi, next, stack1, stack2, flagged);
          if (result) {
            return result;
          }
          stack2.pop_back();
        }
      }
      return std::nullopt;
    }
    
    template <typename S, typename A>
    std::optional<Lasso<S>> dfs1(Buchi<S,A> const& buchi, S const& q, std::vector<S>& stack, auto_set<S>& hashed, auto_set<S>& flagged) {
      hashed.insert(q);
      for (auto& [_,next] : buchi.getTransitions(q)) {
        if (hashed.count(next) == 0) {
          stack.push_back(next);
          auto result = dfs1(buchi, next, stack, hashed, flagged);
          if (result) {
            return result;
          }
          stack.pop_back();
        }
      }
      if (buchi.accepting(q)) {
        std::vector<S> stack2{};
        stack2.emplace_back(q);
        return dfs2(buchi, q, stack, stack2, flagged);
      }
      return std::nullopt;
    }
  }

  // Searches a Buchi automaton for an accepting run. Returns a lasso if one is found.
  // Otherwise returns std::nullopt_t which implies the Buchi's language is empty.
  template <typename S, typename A>
  std::optional<Lasso<S>> FindAcceptingRun(Buchi<S,A> const& buchi) {
    auto_set<S> hashed;
    auto_set<S> flagged;
    for (auto& initState : buchi.getInitialStates()) {
      std::vector<S> stack {};
      stack.emplace_back(initState);
      auto result = _details_::dfs1(buchi, initState, stack, hashed, flagged);
      if (result) {
        return result;
      }
    }
    return std::nullopt;
  }

  // Calculates the intersection of two buchi automata. M must be a functor with bool operator()(A1 const&, A2 const&) that determines if an element of A1 and an element of A2 are a "match".
  template <typename S1, typename S2, typename A1, typename A2, typename M>
  auto Intersection(Buchi<S1,A1> const& b1, Buchi<S2,A2> const& b2, M const& labelMatch) {
    using InterStateType = std::tuple<S1, S2, int>;
    using BuchiType = Buchi<InterStateType, A1>;

    // Initial state construction
    typename BuchiType::StateSet interInitialStates;
    for (auto const& s1 : b1.getInitialStates()) {
      for (auto const& s2 : b2.getInitialStates()) {
        interInitialStates.emplace(s1, s2, 0);
      }
    }

    // Definition of accepting states
    auto interAcceptingStates = [](InterStateType const& s) {
      return std::get<2>(s) == 2;
    };

    // Definition of state transition function
    auto interStateTransitions = [b1,b2,labelMatch](InterStateType const& s) {
      typename BuchiType::TransitionSet transitions;
      int x = std::get<2>(s);

      auto const& b1Trans = b1.getTransitions(std::get<0>(s));
      auto const& b2Trans = b2.getTransitions(std::get<1>(s));

      for (auto const& [label1, head1] : b1Trans) {
        for (auto const& [label2, head2] : b2Trans) {
          if (labelMatch(label1,label2)) {
            int y = x;
            if (x == 0 && b1.accepting(head1)) {
              y = 1;
            } else if (x == 1 && b2.accepting(head2)) {
              y = 2;
            } else if (x == 2) {
              y = 0;
            }
            transitions.emplace(label1, std::make_tuple(head1, head2, y));
          }
        }
      }
      return transitions;
    };

    return BuchiType(interInitialStates, interStateTransitions, interAcceptingStates);
  }

  template <typename S1, typename S2, typename A>
  auto Intersection(Buchi<S1,A> const& b1, Buchi<S2,A> const& b2) {
    return Intersection(b1, b2,std::equal_to<A>{});
  }
}

#endif
