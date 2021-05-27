#ifndef MODEL_CHECK_HH
#define MODEL_CHECK_HH

#include "kripke.hh"
#include "buchi.hh"
#include "ltl.hh"
#include "ltl_normalize.hh"
#include "kripke_to_buchi.hh"
#include "ltl_to_buchi.hh"
#include "buchi_utils.hh"


namespace mc {
  template <typename State, typename AP>
  auto ModelCheck(Kripke<State, AP> const& kripke, ltl::Formula<State, AP> const& spec) {
    auto normalizedSpec = ltl::Normalize(spec);
    auto kripke_buchi = KripkeToBuchi(kripke, normalizedSpec.getAPVec());
    auto ltl_buchi = ltl::LTLToBuchi(normalizedSpec);

    // This functor determines if the set of APs appearing on a transition in ltl_buchi is a subset of the APs appearing on a transition in kripke_buchi. If so then we should be able to take this transition in the intersection.
    auto specAPSubsetKripkeAP = [](auto_set<AP> const& kripkeAPs, auto_set<AP> const& specAPs) {
      for (auto& ap : specAPs) {
        if (kripkeAPs.count(ap) == 0) {
          return false;
        }
      }
      return true;
    };

    auto intersection = Intersection(kripke_buchi, ltl_buchi, specAPSubsetKripkeAP);
    return FindAcceptingRun(intersection);
  }
}

#endif
