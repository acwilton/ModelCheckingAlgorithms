#ifndef SIMPLE_SET_HH
#define SIMPLE_SET_HH

#include <vector>
#include <utility>
#include <functional>

namespace mc {
  /**
   * A class representing a (unordered) set that is compatible with any type which has ==.
   * Only having == makes it hard to implement this class efficiently.
   */
  template <typename T, typename TEqual = std::equal_to<T>>
  class simple_set {
  public:
    // Leaky abstraction here. Reveals what the underlying type is but works for now.
    using iterator = typename std::vector<T>::const_iterator;
    using const_iterator = typename std::vector<T>::const_iterator;

    simple_set() = default;
    simple_set(simple_set const&) = default;
    simple_set(simple_set&&) = default;
    ~simple_set() = default;

    simple_set& operator=(simple_set const&) = default;
    simple_set& operator=(simple_set&&) = default;

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
      return set.end();
    }

    iterator find(const T& value) {
      // Call the const version of find.
      return static_cast<const simple_set*>(this)->find(value);
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
        return std::make_pair(emplaced_iter, true);
      }
      set.erase(emplaced_iter);
      return std::make_pair(matchIter, false);
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
    template <typename F>
    std::pair<iterator, bool> forwarding_insert(F&& value) {
      iterator matchIter = find(value);
      if (matchIter == end()) {
        set.emplace_back(std::forward<F>(value));
        return std::make_pair(set.end()-1, true);
      }
      return std::make_pair(matchIter, false);
    }
    
    std::vector<T> set;
  };
}

#endif
