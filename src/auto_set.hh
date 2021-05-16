#ifndef AUTO_SET_HH
#define AUTO_SET_HH

#include <utility>
#include <functional>
#include <unordered_set>
#include <set>
#include <type_traits>

#include "auto_traits.hh"
#include "simple_set.hh"

namespace mc {
  /**
   * A class representing a (unordered) set that is compatible with most types.
   * The underlying representation for the set is automatically chosen at compile time.
   * If T is hashable then it uses std::unordered_map,
   * else if T is comparable with operator< then it uses std::set,
   * else it chooses the simple_set representation which works with any type with ==.
   */
  template <typename T>
  class auto_set {
  public:
    // The conditional check to determine which underlying representation to use
    using set_representation =
      std::conditional_t<traits::hashable<T>::value, std::unordered_set<T>,
                       std::conditional_t<traits::comparable<T>::value, std::set<T>, simple_set<T>>>;
    using iterator = typename set_representation::iterator;
    using const_iterator = typename set_representation::const_iterator;

    auto_set() = default;
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
      return set.find(value);
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
}

#endif
