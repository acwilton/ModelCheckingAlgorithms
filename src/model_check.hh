#ifndef MODEL_CHECK_HH
#define MODEL_CHECK_HH

#include "kripke.hh"
#include "buchi.hh"
#include "ltl.hh"
#include "ltl_utils.hh"
#include "kripke_to_buchi.hh"
#include "ltl_to_buchi.hh"
#include "buchi_utils.hh"


namespace mc {
  template <typename State, typename AP>
  std::optional<Lasso<State>> ModelCheck(Kripke<State, AP> const& kripke, ltl::Formula<AP> const& normalizedSpec) {
    auto kripke_buchi = KripkeToBuchi(kripke, normalizedSpec.getAPSet());
    auto ltl_buchi = ltl::LTLToBuchi(normalizedSpec);
    using KripkeAlphabet = typename decltype(kripke_buchi)::AlphabetType;
    using LTLAlphabet = typename decltype(ltl_buchi)::AlphabetType;

    // This functor determines if the set of APs appearing on a transition in ltl_buchi is a subset of the APs appearing on a transition in kripke_buchi. If so then we should be able to take this transition in the intersection.
    auto specAPSubsetKripkeAP = [](KripkeAlphabet const& kripkeAPs, LTLAlphabet const& specAPs) {
      for (auto& [truth, ap] : specAPs) {
        bool containsAP = (kripkeAPs.count(ap) == 1);
        if (containsAP != truth) {
          return false;
        }
      }
      return true;
    };
    auto intersection = Intersection(kripke_buchi, ltl_buchi, specAPSubsetKripkeAP);
    auto opt_lasso = FindAcceptingRun(intersection);
    if (opt_lasso) {
      const auto& [bloatedStem, bloatedLoop] = *opt_lasso;
      using StatePair = std::pair<State,ltl::_details_::LTLNode<AP>>;

      // First extract the (Kripke,LTL) state pair.
      // We need LTL included initially so that we don't accidentally trim necessary states in the processing after this phase.
      auto ExtractStatePairString = [](auto const& bloatedStateString) {
        std::vector<StatePair> longStatePairString;
        longStatePairString.reserve(bloatedStateString.size() - 1);

        for (size_t i = 0; i < bloatedStateString.size(); ++i) {
          auto& opt_kState = std::get<0>(std::get<0>(bloatedStateString[i]));
          auto& opt_lState = std::get<0>(std::get<1>(bloatedStateString[i]));
          if (opt_kState || opt_lState) {
            longStatePairString.emplace_back(std::make_pair(*opt_kState,
                                                            *opt_lState));
          }
        }
        return longStatePairString;
      };
      std::vector<StatePair> longStem = ExtractStatePairString(bloatedStem);
      std::vector<StatePair> longLoop = ExtractStatePairString(bloatedLoop);

      // Now we clip any redundant loops that appear within each of the stem and loop.
      auto ClipStatePairString = [](std::vector<StatePair> const& statePairString) {
        std::vector<StatePair> clippedStatePairString;

        for (auto frontIter = statePairString.begin(); frontIter != statePairString.end(); ++frontIter) {
          auto backIter = statePairString.end() - 1;
          for (; backIter != frontIter && *backIter != *frontIter; --backIter) {}
          frontIter = backIter;
          clippedStatePairString.emplace_back(*frontIter);
        }
        return clippedStatePairString;
      };
      std::vector<StatePair> clippedStem = ClipStatePairString(longStem);
      std::vector<StatePair> clippedLoop = ClipStatePairString(longLoop);

      // Now we trim the stem shorter so that there is no overlap between the stem and the loop
      auto stemOverlapIter = clippedStem.end();
      auto loopOverlapIter = clippedLoop.begin();
      for (auto stemIter = clippedStem.begin(); stemIter != clippedStem.end(); ++stemIter) {
        for (auto loopIter = clippedLoop.begin(); loopIter != clippedLoop.end(); ++loopIter) {
          if (*stemIter == *loopIter) {
            stemOverlapIter = stemIter;
            loopOverlapIter = loopIter;
            break;
          }
        }
        if (stemOverlapIter != clippedStem.end()) {
          break;
        }
      }
      std::vector<StatePair> finalStem (clippedStem.begin(), stemOverlapIter);
      std::vector<StatePair> finalLoop (loopOverlapIter, clippedLoop.end());
      finalLoop.insert(finalLoop.end(), clippedLoop.begin(), loopOverlapIter);

      // Finally we extract just the kripke states to be returned.
      auto ExtractKripkeStateString = [](std::vector<StatePair> const& statePairString) {
        std::vector<State> kripkeStateString;
        for (const auto& [kripkeState,ltlState] : statePairString) {
          kripkeStateString.emplace_back(kripkeState);
        }
        return kripkeStateString;
      };

      return std::make_optional(std::make_pair(ExtractKripkeStateString(finalStem),
                                               ExtractKripkeStateString(finalLoop)));
    } else {
      return std::nullopt;
    }
  }
}

#endif
