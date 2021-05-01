#include <unordered_set>
#include <unordered_map>
#include <functional>

template <typename Alphabet>
class Buchi {
public:
  struct Transition {
    Alphabet label;
    int target;
  };

  class State {
  public:
    State(int id, std::vector<Transition> transitions)
      : id(id),
        transitions(transitions)
      {}
    State(State const&) = default;
    State(State&&) = default;
    ~State() = default;

    State& operator=(State const&) = default;
    State& operator=(State&&) = default;

    bool operator==(State const& other) const noexcept {
      return id == other.id;
    }

    int getId() const noexcept {
      return id;
    }
    const std::vector<Transition>& getTransitions() const noexcept {
      return transitions;
    }
    
  private:
    int id;
    std::vector<Transition> transitions;
  };

  using StateSet = std::unordered_map<int, State>;
  using StateSubset = std::unordered_set<int>;
  using StateCharFunc = std::function<bool(State const&)>;


  Buchi(StateSet states, StateSubset initialStates, StateCharFunc acceptingStates)
    : states(states),
      initialStates(initialStates),
      acceptingStates(acceptingStates)
    {}
  
  Buchi(Buchi const&) = default;
  Buchi(Buchi&&) = default;
  ~Buchi() = default;

  Buchi& operator=(Buchi const&) = default;
  Buchi& operator=(Buchi&&) = default;
  
  int size() const noexcept {
    return states.size();
  }

  const StateSet& getStates() const noexcept {
    return states;
  }

  const StateSubset& getInitialStates() const noexcept {
    return initialStates;
  }

  const State& at(int id) const{
    return states.at(id);
  }

  bool initial(State const& state) {
    return initial(state.getId());
  }

  bool initial(int id) {
    return initialStates.find(id) != initialStates.end();
  }

  bool accepting(State const& state) {
    return acceptingStates(state);
  }

  bool accepting(int id) {
    return accepting(states.at(id));
  }

private:
  const StateSet states;
  const StateSubset initialStates;
  const StateCharFunc acceptingStates;
};

/*
namespace std {
  template <typename Alphabet>
  struct hash<Buchi<Alphabet>::State> {
    std::size_t operator()(Buchi<Alphabet>::State const& state) const noexcept {
      return std::hash<int>{}(state.getId());
    }
  };
}
*/
