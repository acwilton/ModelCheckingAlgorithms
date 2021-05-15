#ifndef AUTO_TRAITS_HH
#define AUTO_TRAITS_HH

#include <type_traits>
#include <utility>

namespace mc {
  namespace traits {
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
  }
}

#endif
