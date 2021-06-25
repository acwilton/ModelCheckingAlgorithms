#ifndef BUCHI_PRINTER_HH
#define BUCHI_PRINTER_HH

#include <queue>
#include <utility>
#include <sstream>
#include <string>

#include "auto_map.hh"
#include "auto_set.hh"

namespace mc {

  namespace _details_ {
    template <typename T>
    std::string genericToString(T elem) {
      static auto_map<T,int> seen;
      static int idCount = 0;
      auto iter = seen.find(elem);
      if (iter == seen.end()) {
        seen[elem] = ++idCount;
        return std::to_string(idCount);
      } else {
        auto& [_, id] = *iter;
        return std::to_string(id);
      }
    }
  }

  template <typename S, typename A>
  void PrintBuchi(std::ostream& out, Buchi<S,A> const& buchi,
                  std::function<std::string(A)> alphabetToString = _details_::genericToString<A>,
                  std::function<std::string(S)> stateToString = _details_::genericToString<S>) {
    auto_set<S> closed;
    auto_set<S> open;

    std::stringstream initStream;
    std::stringstream accStream;
    std::stringstream tranStream;

    for (auto const& init : buchi.getInitialStates()) {
      initStream << stateToString(init) << ",";
      open.insert(init);
    }

    while (!open.empty()) {
      auto state = *open.begin();
      open.erase(open.begin());
      closed.insert(state);
      std::string stateStr = stateToString(state);
      if (buchi.accepting(state)) {
        accStream << stateStr << ",";
      }
      for (auto const& [label, head] : buchi.getTransitions(state)) {
        tranStream << "  (" << stateStr << ", " << alphabetToString(label) << ", " << stateToString(head) << ")\n";
        if (closed.count(head) == 0 && open.count(head) == 0) {
          open.insert(head);
        }
      }
    }

    out << "{\n";
    std::string initStr = initStream.str();
    out << "  <" << initStr.substr(0, initStr.size()-1) << ">\n";
    std::string accStr = accStream.str();
    out << "  [" << accStr.substr(0, accStr.size()-1) << "]\n";
    out << tranStream.str();
    out << "}\n";
  }
}
#endif
