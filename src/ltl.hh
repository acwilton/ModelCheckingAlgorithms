#ifndef LTL_HH
#define LTL_HH

#include <variant>
#include <utility>
#include <ostream>
#include <sstream>

#include "auto_set.hh"
#include "eq_function.hh"
#include "kripke.hh"

namespace mc {
  namespace ltl {
    enum class FormulaForm {
      Atomic,
      Not,
      Or,
      And,
      Global,
      Future,
      Until,
      Release
    };
    
    template <typename AP>
    class Formula;

    template <typename AP>
    Formula<AP> make_atomic(AP const& atomic) {
      return Formula<AP>(atomic);
    }

    template <typename F, typename AP>
    Formula<AP> make_atomic(F const& atomic) {
      return make_atomic<AP>(AP(atomic));
    }

    template <typename AP>
    Formula<AP> make_not(Formula<AP> const& sub1) {
      return (sub1.form() == FormulaForm::Atomic)
        ? make_atomic<AP>([ap = sub1.getAP()](auto const& s) {
            return !ap(s);
          })
        : Formula<AP>(FormulaForm::Not, sub1);
    }

    template <typename AP>
    Formula<AP> make_or(Formula<AP> const& sub1, Formula<AP> const& sub2) {
      return (sub1.form() == FormulaForm::Atomic && sub2.form() == FormulaForm::Atomic)
        ? make_atomic<AP>([ap1 = sub1.getAP(), ap2 = sub2.getAP()](auto const& s) {
            return ap1(s) || ap2(s);
          })
        : Formula<AP>(FormulaForm::Or, sub1, sub2);
    }

    template <typename AP>
    Formula<AP> make_and(Formula<AP> const& sub1, Formula<AP> const& sub2) {

      return (sub1.form() == FormulaForm::Atomic && sub2.form() == FormulaForm::Atomic)
        ? make_atomic<AP>([ap1 = sub1.getAP(), ap2 = sub2.getAP()](auto const& s) {
            return ap1(s) && ap2(s);
          })
        : Formula<AP>(FormulaForm::And, sub1, sub2);
    }

    template <typename AP>
    Formula<AP> make_global(Formula<AP> const& sub1) {
      return Formula<AP>(FormulaForm::Global, sub1);
    }

    template <typename AP>
    Formula<AP> make_future(Formula<AP> const& sub1) {
      return Formula<AP>(FormulaForm::Future, sub1);
    }

    template <typename AP>
    Formula<AP> make_until(Formula<AP> const& sub1, Formula<AP> const& sub2) {
      return Formula<AP>(FormulaForm::Until, sub1, sub2);
    }

    template <typename AP>
    Formula<AP> make_release(Formula<AP> const& sub1, Formula<AP> const& sub2) {
      return Formula<AP>(FormulaForm::Release, sub1, sub2);
    }

    template <typename AP>
    class Formula {
    public:
      Formula() = default;
      Formula(Formula const&) = default;
      Formula(Formula&&) = default;

      Formula& operator=(Formula const&) = default;
      Formula& operator=(Formula&&) = default;

      bool operator==(Formula const& rhs) const {
        if (this == &rhs) { return true; }
        return formulaForm == rhs.formulaForm && children == rhs.children;
      }

      bool operator!=(Formula const& rhs) const {
        return !(*this == rhs);
      }

      FormulaForm form() const {
        return formulaForm;
      }

      AP getAP() const {
        if (form() != FormulaForm::Atomic) {
          throw std::domain_error("Attempt to access AP value of non-atomic formula.");
        }
        return std::get<AP>(children);
      }

      auto_set<AP> getAPSet() const {
        return apSet;
      }

      std::vector<Formula>& getSubformulas() {
        if (form() == FormulaForm::Atomic) {
          throw std::domain_error("Attempt to access subformulas of atomic formula.");
        }
        return std::get<std::vector<Formula>>(children);
      }

      std::vector<Formula> const& getSubformulas() const {
        if (form() == FormulaForm::Atomic) {
          throw std::domain_error("Attempt to access subformulas of atomic formula.");
        }
        return std::get<std::vector<Formula>>(children);
      }

      friend Formula make_atomic<AP>(AP const&);
      template <typename F, typename A>
      friend Formula<A> make_atomic(F const&);

      friend Formula make_not<AP>(Formula const&);
      friend Formula make_or<AP>(Formula const&, Formula const&);
      friend Formula make_and<AP>(Formula const&, Formula const&);
      friend Formula make_global<AP>(Formula const&);
      friend Formula make_future<AP>(Formula const&);
      friend Formula make_until<AP>(Formula const&, Formula const&);
      friend Formula make_release<AP>(Formula const&, Formula const&);
    private:
      Formula(AP const& ap)
        : formulaForm(FormulaForm::Atomic),
          children(ap),
          apSet{ap}
        {}
      Formula(FormulaForm formulaForm, Formula sub)
        : formulaForm(formulaForm),
          children(std::vector<Formula>{sub}),
          apSet(sub.apSet)
        {}
      Formula(FormulaForm formulaForm, Formula sub1, Formula sub2)
        : formulaForm(formulaForm),
          children(std::vector<Formula>{sub1, sub2}),
          apSet(sub1.apSet)
        {
          for (auto& ap : sub2.apSet) {
            apSet.insert(ap);
          }
        }

      FormulaForm formulaForm;
      std::variant<std::vector<Formula>,AP> children;
      auto_set<AP> apSet;
    };

    template <typename AP>
    inline const auto False = make_atomic<AP>([](auto const&) { return false; });
    template <typename AP>
    inline const auto True = make_atomic<AP>([](auto const&) { return true; });

    template <typename T>
    std::ostream& operator<< (std::ostream& stream, Formula<T> const& formula) {
      std::stringstream formStringStream;
      formStringStream << "(";
      switch(formula.form()) {
      case FormulaForm::Atomic:
        formStringStream << formula.getAP();
        break;

      case FormulaForm::Until:
        formStringStream << "U " << formula.getSubformulas()[0] << " " << formula.getSubformulas()[1];
        break;

      case FormulaForm::Release:
        formStringStream << "R " << formula.getSubformulas()[0] << " " << formula.getSubformulas()[1];
        break;

      case FormulaForm::Global:
        formStringStream << "G " << formula.getSubformulas()[0];
        break;

      case FormulaForm::Future:
        formStringStream << "F " << formula.getSubformulas()[0];
        break;

      case FormulaForm::Or:
        formStringStream << "|| " << formula.getSubformulas()[0] << " " << formula.getSubformulas()[1];
        break;

      case FormulaForm::And:
        formStringStream << "&& " << formula.getSubformulas()[0] << " " << formula.getSubformulas()[1];
        break;

      case FormulaForm::Not:
        formStringStream << "! " << formula.getSubformulas()[0];
        break;
      }
      formStringStream << ")";
      stream << formStringStream.str();
      return stream;
    }
  }
}

#endif
