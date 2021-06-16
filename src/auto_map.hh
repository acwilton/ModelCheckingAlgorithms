#ifndef AUTO_MAP_HH
#define AUTO_MAP_HH

#include <utility>
#include <functional>
#include <unordered_map>
#include <type_traits>

#include "auto_traits.hh"
#include "simple_map.hh"

namespace mc {
  /**
   * A class representing a (unordered) map that is compatible with most types.
   * The underlying representation for the map is automatically chosen at compile time.
   * If K is hashable then it uses std::unordered_map,
   * else it chooses the simple_map representation which works with any type with ==.
   */
  template <typename K, typename V>
  class auto_map {
  public:
    using value_type = std::pair<const K, V>;
    // The conditional check to determine which underlying representation to use
    using map_representation =
      std::conditional_t<traits::hashable<K>::value, std::unordered_map<K,V>, simple_map<K,V>>;
    using iterator = typename map_representation::iterator;
    using const_iterator = typename map_representation::const_iterator;

    auto_map() = default;
    auto_map(auto_map const&) = default;
    auto_map(auto_map&&) = default;
    auto_map(map_representation const& map) : map(map) {}
    auto_map(map_representation&& map) : map(std::move(map)) {}
    template <typename InputIt>
    auto_map(InputIt first, InputIt last) : map(first, last) {}
    auto_map(std::initializer_list<std::pair<K,V>> init) : map(init) {}

    ~auto_map() = default;

    auto_map& operator=(auto_map const&) = default;
    auto_map& operator=(auto_map&&) = default;
    auto_map& operator=(std::initializer_list<std::pair<K,V>> init) {
      map = init;
    }

    bool operator==(auto_map const& rhs) const {
      return map == rhs.map;
    }
    bool operator!=(auto_map const& rhs) const {
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
      return map.cend();
    }

    iterator find(const K& key) {
      return map.find(key);
    }
    const_iterator find(const K& key) const {
      return map.find(key);
    }

    std::pair<iterator,bool> insert(const value_type& kv) {
      return map.insert(kv);
    }
    std::pair<iterator,bool> insert(value_type&& kv) {
      return map.insert(std::move(kv));
    }

    template <typename... Args>
    std::pair<iterator,bool> emplace(Args&&... args) {
      return map.emplace(std::forward<Args>(args)...);
    }

    iterator erase(const_iterator pos) {
      return map.erase(pos);
    }
    size_t erase(const K& key) {
      return map.erase(key);
    }

    void clear() {
      map.clear();
    }

    V& at(const K& key) {
      return map.at(key);
    }
    const V& at(const K& key) const {
      return map.at(key);
    }

    V& operator[](const K& key) {
      return map[key];
    }
    V& operator[](K&& key) {
      return map[std::move(key)];
    }

    size_t count(const K& key) const {
      return map.count(key);
    }

    size_t size() const {
      return map.size();
    }
    bool empty() const {
      return map.empty();
    }

  private:
    map_representation map;
  };
}

#endif
