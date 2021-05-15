#ifndef AUTO_SET_HH
#define AUTO_SET_HH

#include <vector>
#include <unordered_set>
#include <set>
#include <type_traits>

namespace mc {
  // Forward declaration of simple_set so that auto_set can use it
  template <typename T>
  class simple_set;

  /**
   * A class representing a (unordered) set that is compatible with any type.
   * The underlying representation for the set is automatically chosen at compile time.
   * If T is hashable then it uses std::unordered_map,
   * else if T is comparable with operator< then it uses std::set,
   * else it chooses the simple_set representation which works with any type.
   */
  template <typename T>
  class auto_set {
  private:
    // Compile time check to see if a type supports operator< using SFINAE
    template <typename S, typename = void>
    struct comparable : std::false_type {};
    template <typename S>
    struct comparable<S, std::void_t<decltype(std::declval<S>() < std::declval<S>())>> : std::true_type {};

    // Compile time check to see if a type supports hashing using SFINAE
    template <typename S, typename = void>
    struct hashable : std::false_type {};
    template <typename S>
    struct hashable<S, std::void_t<decltype(std::hash<S>{}(std::declval<S>()))>> : std::true_type {};

    
  public:
    // The conditional check to determine which underlying representation to use
    using set_representation =
      std::conditional<hashable<T>::value, std::unordered_set<T>,
                       std::conditional<comparable<T>::value, std::set<T>, simple_set<T>>>;
    using iterator = set_representation::iterator;
    using const_iterator = set_representation::const_iterator;

    auto_set(auto_set const&) = default;
    auto_set(auto_set&&) = default;
    auto_set(set_representation const& set) : set(set) {}
    auto_set(set_representation&& set) : set(std::move(set)) {}

    ~auto_set() = default;

    auto_set& operator=(auto_set const&) = default;
    auto_set& operator=(auto_set&&) = default;

    iterator begin() {
      return set.begin();
    }
    const_iterator begin() const {
      return set.begin();
    }
    const_iterator cbegin() const {
      return set.cbegin();
    }
    
    iterator end() {
      return set.end();
    }
    const_iterator end() const {
      return set.end();
    }
    const_iterator cend() const {
      return set.cend();
    }

    iterator find(const T& value) {
      return set.find(value);
    }
    const_iterator find(const T& value) const {
      return set.find(value)
    }

    std::pair<iterator,bool> insert(const T& value) {
      return set.insert(value);
    }
    std::pair<iterator,bool> insert(T&& value) {
      return set.insert(std::move(value));
    }

    template <typename... Args>
    std::pair<iterator,bool> emplace(Args&&... args) {
      return set.emplace(std::forward<Args>(args)...);
    }

    iterator erase(const_iterator pos) {
      return set.erase(pos);
    }
    size_t erase(const T& value) {
      return set.erase(value);
    }

    void clear() {
      set.clear();
    }

    size_t count(const T& value) const {
      return set.count(value);
    }

    size_t size() const {
      return set.size();
    }
    bool empty() const {
      return set.empty();
    }

  private:
    set_representation set;
  };


  /**
   * A class representing a (unordered) set that is compatible with any type which has ==.
   * Only having == makes it hard to implement this class efficiently.
   */
  template <typename T, typename TEqual = std::equal_to<T>>
  class simple_set {
  public:
    // Leaky abstraction here. Reveals what the underlying type is but works for now.
    using iterator = std::vector<T>::iterator;
    using const_iterator = std::vector<T>::const_iterator;
    
    simple_set(simple_set const&) = default;
    simple_set(simple_set&&) = default;
    ~simple_set() = default;

    simple_set& operator=(simple_set const&) = default;
    simple_set& operator=(simple_set&&) = default;

    iterator begin() {
      return set.begin();
    }
    iterator begin() const {
      return set.begin();
    }
    iterator cbegin() const {
      return set.cbegin();
    }
    
    iterator end() {
      return set.end();
    }
    const_iterator end() const {
      return set.end();
    }
    const_iterator cend() const {
      return set.end();
    }

    iterator find(const T& value) {
      // Call the const version of find.
      return const_cast<iterator>(static_cast<const simple_set*>(this)->find(value));
    }
    const_iterator find(const T& value) const {
      for (auto iter = cbegin(); iter != cend(); ++iter) {
        if (TEqual{}(*iter, value)) {
          return iter;
        }
      }
      return cend();
    }

    std::pair<iterator,bool> insert(const T& value) {
      return forwarding_insert(value);
    }
    std::pair<iterator,bool> insert(T&& value) {
      return forwarding_insert(std::move(value));
    }

    template <typename... Args>
    std::pair<iterator,bool> emplace(Args&&... args) {
      iterator emplaced_iter = set.emplace(cend(), std::forward<Args>(args)...);
      iterator matchIter = find(*emplaced_iter);
      if (matchIter == emplaced_iter) {
        return std::make_tuple(emplaced_iter, true);
      }
      set.erace(emplaced_iter);
      return (matchIter, false);
    }

    iterator erase(const_iterator pos) {
      return set.erase(pos);
    }
    size_t erase(const T& value) {
      auto matchIter = find(value);
      if (matchIter != end()) {
        erase(matchIter);
        return 1;
      }
      return 0;
    }

    void clear() {
      set.clear();
    }

    size_t count(const T& value) const {
      return find(value) == end() ? 0 : 1;
    }

    size_t size() const {
      return set.size();
    }
    bool empty() const {
      return set.size() == 0;
    }

  private:
    template <tyepename F>
    auto forwarding_insert(F&& value) {
      iterator matchIter = find(value);
      if (matchIter == end()) {
        return std::make_tuple(set.insert(matchIter, std::forward<F>(value)), true);
      }
      return std::make_tuple(matchIter, false);
    }
    
    std::vector<T> set;
  };
}

#endif
