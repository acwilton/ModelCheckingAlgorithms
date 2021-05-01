#include "buchi.hh"

template <typename A>
Buchi<A> intersection(Buchi<A> const& b1, Buchi<A> const& b2) {
  int interectionSize = b1.size() * b2.size() * 3;
  
  Buchi<A>::StateSet intersectionStates;
  intersectionStates.reserve(interectionSize);
  Buchi<A>::StateSubset initialStates;
  Buchi<A>::StateSubset acceptingStates;

  auto targetId = [](int id1, int id2, int x) {
    return id1 * b2.size() * 3 + id2 * 3 + x;
  };

  // First three loops are used to enumerate all triplets (s1, s2, x)
  for (State const& state1 : b1.getStates()) {
    bool s1Initial = b1.initial(state1);
    for (State const& state2 : b2.getStates()) {
      bool s2Initial = b2.initial(state2);
      for (int x = 0; x < 3; ++x) {
        // To generate the set of transitions for a given state (s1, s2, x) we must enumerate all
        // pairs of transitions (t1,t2) where t1 is a transition from s1 and t2 is a transition from s2
        std::vector<Buchi<A>::Transition> newTransitions;
        for (auto const& tran1 : state1.getTransitions()) {
          for (auto const& tran2 : state2.getTransitions()) {
            if (tran1.label == tran2.label) {
              int y = x;
              if (x == 0 && b1.accepting(tran1.target)) {
                y = 1;
              } else if (x == 1 && b2.accepting(tran2.target)) {
                y = 2;
              } else if (x == 2) {
                y = 0;
              }
              newTransitions.emplace_back(tran1.label, targetId(tran1.getId(), tran2.getId(), y));
            }
          }
        }
        int newStateId = targetId(state1.getId(), state2.getId(), x);
        Buchi<A>::State newState (newStateId, std::move(newTransitions));
        intersectionStates.emplace(newStateId, newState);
        if (x == 0 && s1Initial && s2Initial) {
          initialStates.emplace(newStateId);
        }
        if (x == 2) {
          acceptingStates.emplace(newStateId);
        }
      }
    }
  }

  return Buchi<A>(intersectionStates,
                 initialStates,
                 [acceptingStates] (State const& s) {
                   return acceptingStates.find(s.getId()) != acceptingStates.end();
                 });
}
