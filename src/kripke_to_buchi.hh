#ifndef KRIPKE_TO_BUCHI_HH
#define KRIPKE_TO_BUCHI_HH

#include <optional>

#include "kripke.hh"
#include "buchi.hh"

namespace mc {

  template <typename AP, typename State>
  auto KripkeToBuchi(Kripke<AP, State> const& kripke, auto_set<AP> const& apSet) {
    // std::optional<State> is a cheap way to simulate State union {iota}
    // iota is represented by no value (i.e. by std::nullopt)
    using BuchiStateType = std::pair<std::optional<State>, size_t>;
    using BuchiAlphabet = auto_set<AP>;
    using BuchiType = Buchi<BuchiAlphabet, BuchiStateType>;

    // Initial state construction
    auto_set<BuchiStateType> buchiInitialStates;
    buchiInitialStates.emplace(std::make_pair(std::nullopt, 0));

    // Definition of accepting states
    auto buchiAcceptingStates = [N = kripke.getNumConstraints()](BuchiStateType const& s) {
      return std::get<1>(s) == N;
    };

    // Definition of state transition function
    auto buchiStateTransitions = [&kripke,&apSet](BuchiStateType const& s) {
      const auto&[optKripkeState, constraintIndex] = s;

      auto_set<BuchiStateType> nextStates = optKripkeState ?
        kripke.getTransitions(*optKripkeState)
        : kripke.getInitialStates();

      size_t y = constraintIndex;
      BuchiType::TransitionSet transitions;
      for (const auto& next : nextStates) {
        if (constraintIndex == kripke.getNumConstraints()) {
          y = 0;
        } else if (kripke.checkConstraint(constraintIndex, next)) {
          y++;
        }

        transitions.emplace(kripke.getAPSubset(next, apSet),
                            std::make_pair(std::make_optional(next), y));
      }
    };

    return BuchiType(buchiInitialStates, buchiStateTransitions, buchiAcceptingStates);
  }

}

#endif