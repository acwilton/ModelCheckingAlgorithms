#include <unordered_set>
#include <functional>

template <typename Alphabet>
class Buchi {
public:
  using Transition = std::pair<Alphabet, StateSet::iterator>;

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

  using StateSet = std::unordered_set<State>;
  using StateSubset = std::function<bool(State const&)>;


  Buchi(StateSet states, StateSubset initialStates, StateSubset acceptingStates)
    : states(states),
      initialStates(initialStates),
      acceptinStates(acceptingStates)
    {}
  Buchi(Buchi const&) = default;
  Buchi(Buchi&&) = default;
  ~Buchi() = default;

  

private:
  StateSet states;
  StateSubset initialStates;
  StateSubset acceptingStates;
};

namespace std {
  template <typename Alphabet>
  struct hash<Buchi<Alphabet>::State> {
    std::size_t operator()(Buchi<Alphabet>::State const& state) const noexcept {
      return std::hash<int>{}(state.getId());
    }
  };
}
