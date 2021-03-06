#ifndef BUCHI_HH
#define BUCHI_HH

#include <vector>
#include <functional>

#include "auto_set.hh"

namespace mc {

  template <typename State, typename Alphabet>
  class Buchi {
  public:
    using StateType = State;
    using AlphabetType = Alphabet;
    using StateSet = auto_set<State>;
    using TransitionSet = auto_set<std::pair<Alphabet,State>>;
    using StateTransitions = std::function<TransitionSet(State const&)>;
    using StateCharFunc = std::function<bool(State const&)>;


    Buchi(StateSet initialStates, StateTransitions stateTransitions, StateCharFunc acceptingStates)
      : initialStates(initialStates),
        stateTransitions(stateTransitions),
        acceptingStates(acceptingStates)
      {}

    Buchi(Buchi const&) = default;
    Buchi(Buchi&&) = default;
    ~Buchi() = default;

    Buchi& operator=(Buchi const&) = default;
    Buchi& operator=(Buchi&&) = default;


    const StateSet& getInitialStates() const {
      return initialStates;
    }

    bool initial(State const& state) const {
      return static_cast<bool>(initialStates.count(state));
    }

    TransitionSet getTransitions(State const& state) const{
      return stateTransitions(state);
    }

    bool accepting(State const& state) const {
      return acceptingStates(state);
    }

  private:
    StateSet initialStates;
    StateTransitions stateTransitions;
    StateCharFunc acceptingStates;
  };

}
#endif
