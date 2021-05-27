#ifndef LTL_HH
#define LTL_HH

#include <variant>
#include <utility>

#include "auto_set.hh"
#include "eq_function.hh"
#include "kripke.hh"

namespace mc {
  namespace ltl {
    template <typename State, typename AP = EqFunction<bool(State const&)>>
    class Formula;

    template <typename State, typename AP = EqFunction<bool(State const&)>>
    Formula<State, AP> make_atomic(AP const& atomic) {
      return Formula<State, AP>(atomic);
    }

    template <typename F, typename State, typename AP = EqFunction<bool(State const&)>>
    Formula<State, AP> make_atomic(F const& atomic) {
      return make_atomic<State, AP>(AP(atomic));
    }

    template <typename State, typename AP = EqFunction<bool(State const&)>>
    Formula<State, AP> make_not(Formula<State, AP> const& sub1) {
      return (sub1.form() == FormulaForm::Atomic)
        ? make_atomic([&ap = sub1.getAP()](State const& s) {
            return !ap(s);
          })
        : Formula<State, AP>(FormulaForm::Not, sub1);
    }

    template <typename State, typename AP = EqFunction<bool(State const&)>>
    Formula<State, AP> make_or(Formula<State, AP> const& sub1, Formula<State, AP> const& sub2) {
      return (sub1.form() == FromulaForm::Atomic && sub2.form() == FormulaForm::Atomic)
        ? make_atomic([&ap1 = sub1.getAP(), &ap2 = sub2.getAP()](State const& s) {
            return ap1(s) || ap2(s);
          })
        : Formula<State, AP>(FormulaForm::Or, sub1, sub2);
    }

    template <typename State, typename AP = EqFunction<bool(State const&)>>
    Formula<State, AP> make_and(Formula<State, AP> const& sub1, Formula<State, AP> const& sub2) {

      return (sub1.form() == FromulaForm::Atomic && sub2.form() == FormulaForm::Atomic)
        ? make_atomic([&ap1 = sub1.getAP(), &ap2 = sub2.getAP()](State const& s) {
            return ap1(s) && ap2(s);
          })
        : Formula<State, AP>(FormulaForm::And, sub1, sub2);
    }

    template <typename State, typename AP = EqFunction<bool(State const&)>>
    Formula<State, AP> make_global(Formula<State, AP> const& sub1) {
      return Formula<State, AP>(FormulaForm::Global, sub1);
    }

    template <typename State, typename AP = EqFunction<bool(State const&)>>
    Formula<State, AP> make_future(Formula<State, AP> const& sub1) {
      return Formula<State, AP>(FormulaForm::Future, sub1);
    }

    template <typename State, typename AP = EqFunction<bool(State const&)>>
    Formula<State, AP> make_until(Formula<State, AP> const& sub1, Formula<State, AP> const& sub2) {
      return Formula<State, AP>(FormulaForm::Until, sub1, sub2);
    }

    template <typename State, typename AP = EqFunction<bool(State const&)>>
    Formula<State, AP> make_release(Formula<State, AP> const& sub1, Formula<State, AP> const& sub2) {
      return Formula<State, AP>(FormulaForm::Release, sub1, sub2);
    }

    enum class FormulaForm = {
      Atomic,
      Not,
      Or,
      And,
      Global,
      Future,
      Until,
      Release
    };

    template <typename State, typename AP>
    class Formula {
    public:
      Formula(Formula const&) = default;
      Formula(Formula&&) = default;

      Formula& operator=(Formula const&) = default;
      Formula& operator=(Formula&&) = default;

      bool operator==(Formula const& rhs) {
        if (this == &rhs) { return true; }
        return form == rhs.form && children == rhs.children;
      }

      bool operator!=(Formula const& rhs) {
        return !(*this == rhs);
      }

      FormulaForm form() const {
        return form;
      }

      AP getAP() const {
        if (form() != FormulaForm::Atomic) {
          throw std::bad_variant_access("Attempt to access AP value of non-atomic formula.");
        }
        return std::get<AP>(children);
      }

      auto_set<AP> getAPSet() const {
        return apSet;
      }

      std::vector<Formula>& getSubformulas() {
        if (form() == FormulaForm::Atomic) {
          throw std::bad_variant_access("Attempt to access subformulas of atomic formula.");
        }
        return std::get<std::vector<Formula>>(children);
      }


      friend Formula make_atomic<State, AP>(AP const&);
      template <typename F, typename S, typename A>
      friend Formula<F, S> make_atomic(F const&);

      friend Formula make_not<State, AP>(Formula const&);
      friend Formula make_or<State, AP>(Formula const&, Formula const&);
      friend Formula make_and<State, AP>(Formula const&, Formula const&);
      friend Formula make_global<State, AP>(Formula const&);
      friend Formula make_future<State, AP>(Formula const&);
      friend Formula make_until<State, AP>(Formula const&, Formula const&);
      friend Formula make_release<State, AP>(Formula const&, Formula const&);
    private:
      Formula(AP const& ap)
        : form(FormulaForm::Atomic),
          children(ap),
          apSet{ap}
        {}
      Formula(FormulaForm form, Formula sub)
        : form(form),
          children(std::vector<Formula>{sub}),
          apSet(sub.apSet)
        {}
      Formula(FormulaForm form, Formula sub1, Formula sub2)
        : form(form),
          children(std::vector<Formula>{sub1, sub2}),
          apSet(sub1.apSet)
        {
          for (auto& ap : sub2.apSet) {
            apSet.insert(ap);
          }
        }

      Formula() = delete;

      FormulaForm form;
      std::variant<std::vector<Formula>,AP> children;
      auto_set<AP> apSet;
    };

    template <typename State, typename AP>
    inline const auto False = make_atomic<State, AP>([](auto const&) { return false; });
    template <typename State, typename AP>
    inline const auto True = make_atomic<State, AP>([](auto const&) { return true; });
  }
}

#endif
