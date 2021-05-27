#ifndef LTL_NORMALIZE_HH
#define LTL_NORMALIZE_HH

#include "ltl.hh";

namespace mc {
  namespace ltl {
    // Puts formula into NNF and removes G's and F's using equivalences to R and U
    template <typename State, typename AP>
    Formula<State, AP> Normalize(Formula<State, AP> const& formula) {
      auto True = ltl::True<State, AP>;
      auto False = ltl::False<State, AP>;

      switch(formula.form()) {
      case FormulaForm::Atomic:
        return formula;

      case FormulaForm::Until:
        return make_until(Normalize(formula.getSubformulas()[0]),
                          Normalize(formula.getSubformulas()[1]));

      case FormulaForm::Release:
        return make_release(Normalize(formula.getSubformulas()[0]),
                            Normalize(formula.getSubformulas()[1]));

      case FormulaForm::Global:
        // (G f) = (R false f)
        return make_release(False, Normalize(formula.getSubformulas()[0]));

      case FormulaForm::Future:
        // (F f) = (U true f)
        return make_until(True, Normalize(formula.getSubformulas()[0]));

      case FormulaForm::Or:
        return make_or(Normalize(formula.getSubformulas()[0]),
                       Normalize(formula.getSubformulas()[1]));

      case FormulaForm::And:
        return make_and(Normalize(formula.getSubformulas()[0]),
                        Normalize(formula.getSubformulas()[1]));

      case FormulaForm::Not:
        auto sub = formula.getSubformulas()[0];

        switch (sub.form()) {
        case FormulaForm::Atomic:
          return formula;

        case FormulaForm::Until:
          return make_release(Normalize(make_not(sub.getSubformulas[0])),
                              Normalize(make_not(sub.getSubformulas[1])));

        case FormulaForm::Release:
          return make_until(Normalize(make_not(sub.getSubformulas[0])),
                            Normalize(make_not(sub.getSubformulas[1])));

        case FormulaForm::Global:
          // (! (G f)) = (F (! f)) = (U true (! f))
          return make_until(True, Normalize(make_not(sub.getSubFormulas()[0])));

        case FormulaForm::Future:
          // (! (F f)) = (G (! f)) = (R false (! f))
          return make_release(False, Normalize(make_not(sub.getSubFormulas()[0])));

        case FormulaForm::Or:
          return make_and(Normalize(make_not(sub.getSubFormulas()[0])),
                          Normalize(make_not(sub.getSubFormulas()[1])));

        case FormulaForm::Or:
          return make_or(Normalize(make_not(sub.getSubFormulas()[0])),
                         Normalize(make_not(sub.getSubFormulas()[1])));

        case FormulaForm::Not:
          return Normalize(sub.getSubformulas()[0]);
        }
      }
    }
  }
}

#endif
