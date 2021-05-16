#include "buchi.hh"

#ifndef BUCHI_INTERSECTION_HH
#define BUCHI_INTERSECTION_HH

namespace mc {

  template <typename A, typename S1, typename S2>
  auto intersection(Buchi<A,S1> const& b1, Buchi<A,S2> const& b2) {
    using InterStateType = std::tuple<S1, S2, const int>;

    // Initial state construction
    auto_set<InterStateType> interInitialStates;
    for (auto const& s1 : b1.getInitialStates()) {
      for (auto const& s2 : b2.getInitialStates()) {
        interInitialStates.emplace_back(s1, s2, 0);
      }
    }

    // Definition of accepting states
    auto interAcceptingStates = [](InterStateType const& s) {
      return std::get<2>(s) == 2;
    };

    // Definition of state transition function
    auto interStateTransitions = [&b1,&b2](InterStateType const& s) {
      auto_map<A, InterStateType> transitions;
      int x = std::get<2>(s);

      auto const& b1Trans = b1.getTransitions(std::get<0>(s));
      auto const& b2Trans = b2.getTransitions(std::get<1>(s));

      for (auto const& [label, head] : b1Trans) {
        auto matchTran = b2Trans.find(label);
        if (matchTran != b2Trans.end()) {
          int y = x;
          if (x == 0 && b1.accepting(head)) {
            y = 1;
          } else if (x == 1 && b2.accepting(matchTran->second)) {
            y = 2;
          } else if (x == 2) {
            y = 0;
          }
          transitions[label] = std::make_tuple(head, matchTran->second, y);
        }
      }
      return transitions;
    };

    return Buchi<A, InterStateType>(interInitialStates, interStateTransitions, interAcceptingStates);
  }

}

#endif
