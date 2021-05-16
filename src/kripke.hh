#ifndef KRIPKE_HH
#define KRIPKE_HH

#include "auto_set.hh"

namespace mc {

  template <typename AP, typename State>
  class Kripke {
  public:
    using StateSet = auto_set<State>;
    using StateTransitions = std::function<auto_set<State>(State const&)>;
    using LabelFunc = std::function<bool(State const&, AP const&)>;
    using StateCharFunc = std::function<bool(State const&)>;


    Kripke(StateSet initialStates,
           StateTransitions stateTransitions,
           LabelFunc labelingFunction,
           std::vector<StateCharFunc> fairnessConstraints = {[](State const&) { return true; }})
      : initialStates(initialStates),
        stateTransitions(stateTransitions),
        labelingFunction(labelingFunction),
        fairnessConstraints(fairnessConstraints)
      {}

    Kripke(Kripke const&) = default;
    Kripke(Kripke&&) = default;
    ~Kripke() = default;

    Kripke& operator=(Kripke const&) = default;
    Kripke& operator=(Kripke&&) = default;

  
    const StateSet& getInitialStates() const {
      return initialStates;
    }

    bool initial(State const& state) const {
      return static_cast<bool>(initialStates.count(state));
    }

    auto_set<State> getTransitions(State const& state) const{
      return stateTransitions(state);
    }

    bool checkAP(State const& state, AP const& ap) const {
      return labelingFunction(state, ap);
    }

    auto_set<AP> getAPSubset(State const& state, auto_set<AP> const& labelSet) {
      auto_set<AP> validAPs;
      for (const auto& ap : labelSet) {
        if (checkAtomicProp(state, ap)) {
          validAPs.insert(ap);
        }
      }
      return validAPs;
    }

    size_t getNumConstraints() const {
      return fairnessConstraints.size();
    }
    bool checkConstraint(int constraintNumber, State const& state) const {
      return fairnessConstraints[constraintNumber](state);
    }

  private:
    const StateSet initialStates;
    const StateTransitions stateTransitions;
    const LabelFunc labelingFunction;
    const std::vector<StateCharFunc> fairnessConstraints;
  };

}

#endif
