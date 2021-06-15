#ifndef EQ_FUNCTION_HH
#define EQ_FUNCTION_HH

#include <functional>
#include <ostream>

namespace mc {
  // Class is undefined if its type is not a function signature
  template <typename T>
  class EqFunction;

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
        f(f),
        strRep(std::to_string(id))
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

    R operator()(Args... args) const {
      return f(args...);
    }

    explicit operator bool() const {
      return static_cast<bool>(f);
    }

    void setRepresentation(std::string newRep) {
      strRep = newRep;
    }

    std::string getRepresentation() const {
      return strRep;
    }

    template <typename F>
    friend std::ostream& operator<<(std::ostream& stream, EqFunction<F> const& func);
  private:
    inline static int count = 0;

    int id;
    std::string strRep;
    std::function<R(Args...)> f;
  };

  template <typename T>
  std::ostream& operator<<(std::ostream& stream, EqFunction<T> const& func) {
    stream << func.getRepresentation();
    return stream;
  }
}

#endif
