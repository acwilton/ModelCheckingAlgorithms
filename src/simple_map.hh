#ifndef SIMPLE_MAP_HH
#define SIMPLE_MAP_HH

#include <vector>
#include <initializer_list>
#include <utility>
#include <functional>
#include <variant>

#include "simple_set.hh"

namespace mc {
  /**
   * A class representing a (unordered) map that is compatible with any key which has ==.
   * Only having == makes it hard to implement this class efficiently.
   */
  template <typename K, typename V, typename KEqual = std::equal_to<K>>
  class simple_map {
  private:
    using underlying_representation = std::vector<std::pair<K,V>>;
    
  public:
    using value_type = std::pair<const K, V>;
    // Leaky abstraction here. Reveals what the underlying type is but works for now.
    using iterator = typename underlying_representation::iterator;
    using const_iterator = typename underlying_representation::const_iterator;

    simple_map() = default;
    simple_map(simple_map const&) = default;
    simple_map(simple_map&&) = default;
    template <typename InputIt>
    simple_map(InputIt first, InputIt last) : map(first, last) {}
    simple_map(std::initializer_list<std::pair<K,V>> init) : map(init) {}
    ~simple_map() = default;

    simple_map& operator=(simple_map const&) = default;
    simple_map& operator=(simple_map&&) = default;
    simple_map& operator=(std::initializer_list<std::pair<K,V>> init) {
      map = init;
    }

    bool operator==(simple_map const& rhs) {
      return map == rhs.map;
    }
    bool operator!=(simple_map const& rhs) {
      return map != rhs.map;
    }

    iterator begin() {
      return map.begin();
    }
    const_iterator begin() const {
      return map.begin();
    }
    const_iterator cbegin() const {
      return map.cbegin();
    }
    
    iterator end() {
      return map.end();
    }
    const_iterator end() const {
      return map.end();
    }
    const_iterator cend() const {
      return map.end();
    }

    iterator find(const K& key) {
      return remove_iterator_constness(static_cast<const simple_map*>(this)->find(key));
    }
    const_iterator find(const K& key) const {
      for (auto iter = cbegin(); iter != cend(); ++iter) {
        if (KEqual{}(iter->first, key)) {
          return iter;
        }
      }
      return cend();
    }

    std::pair<iterator,bool> insert(const value_type& kv) {
      return forwarding_insert(kv);
    }
    std::pair<iterator,bool> insert(value_type&& kv) {
      return forwarding_insert(std::move(kv));
    }

    template <typename... Args>
    std::pair<iterator,bool> emplace(Args&&... args) {
      iterator emplaced_iter = map.emplace(cend(), std::forward<Args>(args)...);
      iterator matchIter = find(emplaced_iter->first);
      if (matchIter == emplaced_iter) {
        return std::make_pair(emplaced_iter, true);
      }
      map.erase(emplaced_iter);
      return std::make_pair(matchIter, false);
    }

    iterator erase(const_iterator pos) {
      return map.erase(pos);
    }
    size_t erase(const K& key) {
      auto matchIter = find(key);
      if (matchIter != end()) {
        erase(matchIter);
        return 1;
      }
      return 0;
    }

    void clear() {
      map.clear();
    }

    V& at(const K& key) {
      return const_cast<V&>(static_cast<const simple_map*>(this)->at(key));
    }
    const V& at(const K& key) const {
      const_iterator match = find(key);
      if (match == end()) {
        throw std::out_of_range("Attempt to access value at a key that does not exist");
      }
      return match->second;
    }
    

    V& operator[](const K& key) {
      return forwarding_access(key);
    }
    V& operator[](K&& key) {
      return forwarding_access(std::move(key));
    }

    size_t count(const K& key) const {
      return find(key) == end() ? 0 : 1;
    }

    size_t size() const {
      return map.size();
    }
    bool empty() const {
      return map.empty();
    }

  private:
    std::pair<K,V> value_type_conversion(value_type const& kv) {
      return std::make_pair(kv.first, kv.second);
    }
    std::pair<K,V> value_type_conversion(value_type&& kv) {
      return std::make_pair(std::move(kv.first), std::move(kv.second));
    }

    template <typename F>
    std::pair<iterator, bool> forwarding_insert(F&& forward_kv) {
      iterator matchIter = find(forward_kv.first);
      if (matchIter == end()) {
        map.emplace_back(value_type_conversion(std::forward<F>(forward_kv)));
        return std::make_pair(map.end()-1, true);
      }
      return std::make_pair(matchIter, false);
    }
    
    template <typename F>
    V& forwarding_access(F&& forward_key) {
      return insert(std::make_pair(std::forward<F>(forward_key), V{})).first->second;
    }

    iterator remove_iterator_constness(const_iterator it) {
      return map.erase(it,it);
    }

    underlying_representation map;
  };
}

#endif
