#ifndef EQ_FUNCTION_HH
#define EQ_FUNCTION_HH

#include <functional>

namespace mc {
  // Class is undefined if its type is not a function signature
  template <typename T>
  class EqFunction;

  // REMOVE THIS COMMENT LATER: Don't augment to allow string == comparison. In parser, use string we would have used for == as a key into a map that maps to the appropriate EqFunction (of course if it doesn't exist yet then make it!)
  template <typename R, typename... Args>
  class EqFunction<R(Args...)> {
  public:
    EqFunction() = default;
    EqFunction(std::nullptr_t) : EqFunction() {}
    EqFunction(EqFunction const&) = default;
    EqFunction(EqFunction&&) = default;

    // Constructs a function with a brand new id different from all other EqFunctions with this signature
    template <typename F>
    EqFunction(F f)
      : id(++count),
        f(f)
      {}

    EqFunction& operator=(EqFunction const&) = default;
    EqFunction& operator=(EqFunction&&) = default;

    // Equality is determined by the id given on construction.
    bool operator==(EqFunction const& rhs) const {
      return id == rhs.id;
    }

    bool operator!=(EqFunction const& rhs) const {
      return !(*this == rhs);
    }

    R operator()(Args... args) {
      return f(args...);
    }

    explicit operator bool() const {
      return static_cast<bool>(f);
    }
  private:
    inline static int count = 0;

    int id;
    std::function<R(Args...)> f;
  };
}

#endif
