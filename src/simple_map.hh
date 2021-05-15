#ifndef SIMPLE_MAP_HH
#define SIMPLE_MAP_HH

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
  public:
    using value_type = std::pair<const K, V>;
  private:
    struct KeyEquality {
      bool operator()(const value_type& kv1, const value_type& kv2) {
        return KEqual{}(kv1.first, kv2.first);
      }
    };
    using underlying_representation = simple_set<value_type, KeyEquality>;
    
  public:
    // Leaky abstraction here. Reveals what the underlying type is but works for now.
    using iterator = underlying_representation::iterator;
    using const_iterator = underlying_representation::const_iterator;
    
    simple_map(simple_map const&) = default;
    simple_map(simple_map&&) = default;
    ~simple_map() = default;

    simple_map& operator=(simple_map const&) = default;
    simple_map& operator=(simple_map&&) = default;

    iterator begin() {
      return map.begin();
    }
    iterator begin() const {
      return map.begin();
    }
    iterator cbegin() const {
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
      return map.find(std::make_pair(key, V{}));
    }
    const_iterator find(const K& key) const {
      return map.find(std::make_pair(key, V{}));
    }

    std::pair<iterator,bool> insert(const value_type& kv) {
      return map.insert(kv);
    }
    std::pair<iterator,bool> insert(value_type&& kv) {
      return map.insert(std::move(kv));
    }

    template <typename... Args>
    std::pair<iterator,bool> emplace(Args&&... args) {
      map.emplace(std::forward<Args>(args)...);
    }

    iterator erase(const_iterator pos) {
      return map.erase(pos);
    }
    size_t erase(const K& key) {
      map.erase(std::make_pair(key, V{}));
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
      return map.count(std::make_tuple(key, V{}));
    }

    size_t size() const {
      return map.size();
    }
    bool empty() const {
      return map.empty();
    }

  private:
    template <typename F>
    V& forwarding_access(F&& forward_key) {
      return map.insert(std::forward<F>(key)).first->second;
    }

    underlying_representation map;
  };
}

#endif
