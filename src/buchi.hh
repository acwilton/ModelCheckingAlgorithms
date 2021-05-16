#ifndef BUCHI_HH
#define BUCHI_HH

#include <vector>
#include <map>
#include <functional>

#include "auto_set.hh"

namespace mc {

  template <typename Alphabet, typename State>
  class Buchi {
  public:
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
    const StateSet initialStates;
    const StateTransitions stateTransitions;
    const StateCharFunc acceptingStates;
  };

}
#endif
