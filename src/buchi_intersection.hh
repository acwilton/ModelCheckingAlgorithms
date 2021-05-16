#include "buchi.hh"

#ifndef BUCHI_INTERSECTION_HH
#define BUCHI_INTERSECTION_HH

namespace mc {

  template <typename A, typename S1, typename S2>
  auto Intersection(Buchi<A,S1> const& b1, Buchi<A,S2> const& b2) {
    using InterStateType = std::tuple<S1, S2, const int>;
    using BuchiType = Buchi<A, InterStateType>;

    // Initial state construction
    BuchiType::StateSet interInitialStates;
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
    auto interStateTransitions = [&b1,&b2](InterStateType const& s) {
      BuchiType::TransitionSet transitions;
      int x = std::get<2>(s);

      auto const& b1Trans = b1.getTransitions(std::get<0>(s));
      auto const& b2Trans = b2.getTransitions(std::get<1>(s));

      for (auto const& [label1, head1] : b1Trans) {
        for (auto const& [label2, head2] : b2Trans) {
          if (label1 == label2) {
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

}

#endif
