#ifndef LTL_UTILS_HH
#define LTL_UTILS_HH

#include <functional>

#include "ltl.hh"

namespace mc {
  namespace ltl {
    // Puts formula into NNF and removes G's and F's using equivalences to R and U
    template <typename AP>
    Formula<AP> Normalize(Formula<AP> const& formula,
                          AP const& trueAP = [](auto const&) { return true; },
                          AP const& falseAP = [](auto const&) { return false; }) {
      auto True = make_atomic<AP>(trueAP);
      auto False = make_atomic<AP>(falseAP);

      switch(formula.form()) {
      case FormulaForm::Atomic:
        return formula;

      case FormulaForm::Until:
        return make_until(Normalize(formula.getSubformulas()[0], trueAP, falseAP),
                          Normalize(formula.getSubformulas()[1], trueAP, falseAP));

      case FormulaForm::Release:
        return make_release(Normalize(formula.getSubformulas()[0], trueAP, falseAP),
                            Normalize(formula.getSubformulas()[1], trueAP, falseAP));

      case FormulaForm::Global:
        // (G f) = (R false f)
        return make_release(False, Normalize(formula.getSubformulas()[0], trueAP, falseAP));

      case FormulaForm::Future:
        // (F f) = (U true f)
        return make_until(True, Normalize(formula.getSubformulas()[0], trueAP, falseAP));

      case FormulaForm::Or:
        return make_or(Normalize(formula.getSubformulas()[0], trueAP, falseAP),
                       Normalize(formula.getSubformulas()[1], trueAP, falseAP));

      case FormulaForm::And:
        return make_and(Normalize(formula.getSubformulas()[0], trueAP, falseAP),
                        Normalize(formula.getSubformulas()[1], trueAP, falseAP));

      case FormulaForm::Not:
        auto sub = formula.getSubformulas()[0];

        switch (sub.form()) {
        case FormulaForm::Atomic:
          return formula;

        case FormulaForm::Until:
          return make_release(Normalize(make_not(sub.getSubformulas()[0]), trueAP, falseAP),
                              Normalize(make_not(sub.getSubformulas()[1]), trueAP, falseAP));

        case FormulaForm::Release:
          return make_until(Normalize(make_not(sub.getSubformulas()[0]), trueAP, falseAP),
                            Normalize(make_not(sub.getSubformulas()[1]), trueAP, falseAP));

        case FormulaForm::Global:
          // (! (G f)) = (F (! f)) = (U true (! f))
          return make_until(True, Normalize(make_not(sub.getSubformulas()[0]), trueAP, falseAP));

        case FormulaForm::Future:
          // (! (F f)) = (G (! f)) = (R false (! f))
          return make_release(False, Normalize(make_not(sub.getSubformulas()[0]), trueAP, falseAP));

        case FormulaForm::And:
          return make_or(Normalize(make_not(sub.getSubformulas()[0]), trueAP, falseAP),
                         Normalize(make_not(sub.getSubformulas()[1]), trueAP, falseAP));

        case FormulaForm::Or:
          return make_and(Normalize(make_not(sub.getSubformulas()[0]), trueAP, falseAP),
                          Normalize(make_not(sub.getSubformulas()[1]), trueAP, falseAP));

        case FormulaForm::Not:
          return Normalize(sub.getSubformulas()[0], trueAP, falseAP);
        }
      }
    }

    template <typename AP>
    Formula<AP> Compress(Formula<AP> const& formula,
                         std::function<AP(AP)> notComp
                         = [](AP ap) {
                           return [ap](auto const& s) {
                             return !ap(s);
                           };
                         },
                         std::function<AP(AP,AP)> orComp
                         = [](AP ap1, AP ap2) {
                           return [ap1, ap2](auto const& s) {
                             return ap1(s) || ap2(s);
                           };
                         },
                         std::function<AP(AP,AP)> andComp
                         = [](AP ap1, AP ap2) {
                           return [ap1, ap2](auto const& s) {
                             return ap1(s) && ap2(s);
                           };
                         })
    {
      switch(formula.form()) {
      case FormulaForm::Atomic:
        return formula;

      case FormulaForm::Not: {
        auto compSub = Compress(formula.getSubformulas()[0], notComp, orComp, andComp);
        return (compSub.form() == FormulaForm::Atomic)
          ? make_atomic<AP>(notComp(compSub.getAP()))
          : make_not<AP>(compSub);
      }

      case FormulaForm::Or: {
        auto compSub1 = Compress(formula.getSubformulas()[0], notComp, orComp, andComp);
        auto compSub2 = Compress(formula.getSubformulas()[1], notComp, orComp, andComp);
        return (compSub1.form() == FormulaForm::Atomic
                && compSub2.form() == FormulaForm::Atomic)
          ? make_atomic<AP>(orComp(compSub1.getAP(), compSub2.getAP()))
          : make_or(compSub1, compSub2);
      }

      case FormulaForm::And: {
        auto compSub1 = Compress(formula.getSubformulas()[0], notComp, orComp, andComp);
        auto compSub2 = Compress(formula.getSubformulas()[1], notComp, orComp, andComp);
        return (compSub1.form() == FormulaForm::Atomic
                && compSub2.form() == FormulaForm::Atomic)
          ? make_atomic<AP>(andComp(compSub1.getAP(), compSub2.getAP()))
          : make_and(compSub1, compSub2);
      }

      case FormulaForm::Until: {
        return make_until(Compress(formula.getSubformulas()[0], notComp, orComp, andComp),
                          Compress(formula.getSubformulas()[1], notComp, orComp, andComp));
      }

      case FormulaForm::Release: {
        return make_release(Compress(formula.getSubformulas()[0], notComp, orComp, andComp),
                            Compress(formula.getSubformulas()[1], notComp, orComp, andComp));
      }

      case FormulaForm::Global: {
        return make_global(Compress(formula.getSubformulas()[0], notComp, orComp, andComp));
      }

      case FormulaForm::Future: {
        return make_future(Compress(formula.getSubformulas()[0], notComp, orComp, andComp));
      }
      }
    }
  }
}

#endif
