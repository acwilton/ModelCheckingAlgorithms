#ifndef KRIPKE_HH
#define KRIPKE_HH

#include "auto_set.hh"
#include "eq_function.hh"

namespace mc {

  template <typename State, typename AP = EqFunction<bool(State const&)>>
  class Kripke {
  public:
    using StateSet = auto_set<State>;
    using StateTransitions = std::function<auto_set<State>(State const&)>;
    using StateCharFunc = std::function<bool(State const&)>;
    using LabelingFunc = std::function<bool(State const&, AP const&)>;


    Kripke(StateSet initialStates,
           StateTransitions stateTransitions,
           std::vector<StateCharFunc> fairnessConstraints = { [](State const&) {
             return true;
           } },
           LabelingFunc labelingFunction = [](State const& s, AP const& ap) {
             return ap(s);
           })
      : initialStates(initialStates),
        stateTransitions(stateTransitions),
        fairnessConstraints(fairnessConstraints),
        labelingFunction(labelingFunction)
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

    auto_set<State> getTransitions(State const& state) const {
      return stateTransitions(state);
    }

    bool checkAP(State const& s, AP const& ap) const {
      return labelingFunction(s,ap);
    }

    auto_set<AP> getAPSubset(State const& state, auto_set<AP> const& labelSet) const {
      std::vector<AP> validAPs;
      for (const auto& ap : labelSet) {
        if (checkAP(state, ap)) {
          validAPs.emplace_back(ap);
        }
      }
      return auto_set<AP>(validAPs.begin(), validAPs.end());
    }

    size_t getNumConstraints() const {
      return fairnessConstraints.size();
    }
    bool checkConstraint(int constraintNumber, State const& state) const {
      return fairnessConstraints[constraintNumber](state);
    }

  private:
    StateSet initialStates;
    StateTransitions stateTransitions;
    std::vector<StateCharFunc> fairnessConstraints;
    LabelingFunc labelingFunction;
  };

}

#endif
