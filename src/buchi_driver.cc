#include <iostream>
#include <fstream>
#include <string>
#include <regex>
#include <functional>
#include <utility>
#include <vector>

#include "buchi.hh"
#include "buchi_utils.hh"
#include "auto_set.hh"
#include "auto_map.hh"


class BuchiParser {
public:
  using Buchi = mc::Buchi<std::string, std::string>;

  BuchiParser(const char* filename)
    : stream(),
      currentLine(""),
      currentLineNum(0),
      parseSuccessful(true)
    {
      stream.open(filename);
      if (!stream.good()) {
        std::cout << "Failed to open file \"" << filename << "\".\n";
        parseSuccessful = false;
      }
    }
  ~BuchiParser() {
    if (stream.is_open()) {
      stream.close();
    }
  }

  BuchiParser() = delete;
  BuchiParser(BuchiParser const&) = delete;
  BuchiParser(BuchiParser&&) = delete;
  BuchiParser& operator=(BuchiParser const&) = delete;
  BuchiParser& operator=(BuchiParser&&) = delete;

  bool ParseSuccessful() {
    return parseSuccessful;
  }

  std::optional<std::pair<Buchi, Buchi>> Parse() {
    if (parseSuccessful) {
      bool buchisExist = true;
      auto opt_buchi1 = match_buchi();
      if (!opt_buchi1) {
        reportError("Could not parse first buchi.");
        return std::nullopt;
      }
      auto opt_buchi2 = match_buchi();
      if (!opt_buchi2) {
        reportError("Could not parse second buchi.");
        return std::nullopt;
      }
      eat_whitespace();
      if (currentLine != "") {
        std::cout << "File has more contents than two Buchi automata. Ignoring all contents after the first two parse Buchi automata.\n";
      }
      return parseSuccessful ? std::make_optional(std::make_pair(*opt_buchi1, *opt_buchi2)) : std::nullopt;
    }
    return std::nullopt;
  }
private:
  // Tokens
  static constexpr auto LPAREN = R"(\()";
  static constexpr auto RPAREN = R"(\))";
  static constexpr auto LBRACE = R"(\{)";
  static constexpr auto RBRACE = R"(\})";
  static constexpr auto LANGLE = R"(\<)";
  static constexpr auto RANGLE = R"(\>)";
  static constexpr auto LSQUARE = R"(\[)";
  static constexpr auto RSQUARE = R"(\])";
  static constexpr auto COMMA = R"(,)";
  static constexpr auto STR = R"([^,\(\)\{\}\[\]\<\>]*[^\s,\(\)\{\}\[\]\<\>])";

  std::optional<mc::auto_set<std::string>> match_buchi_init_states() {
    mc::auto_set<std::string> initStates;
    if (!match_token(LANGLE)) {
      reportError("Expected < at start of initial state specification.");
      return std::nullopt;
    }

    std::string stateMatch;
    if (match_token (stateMatch, STR)) {
      initStates.insert(stateMatch);
      while (match_token (COMMA)) {
        if (match_token(stateMatch, STR)) {
          initStates.insert(stateMatch);
        } else {
          reportError("Expected non-empty string after \",\" in initial state list.");
          return std::nullopt;
        }
      }
    }

    if (!match_token(RANGLE)) {
      reportError("Expected > at end of initial state specification.");
      return std::nullopt;
    }

    return std::make_optional(initStates);
  }

  std::optional<mc::auto_set<std::string>> match_buchi_accept_states() {
    mc::auto_set<std::string> acceptStates;
    if (!match_token(LSQUARE)) {
      reportError("Expected [ at start of accepting state specification.");
      return std::nullopt;
    }

    std::string stateMatch;
    if (match_token (stateMatch, STR)) {
      acceptStates.insert(stateMatch);
      while (match_token (COMMA)) {
        if (match_token(stateMatch, STR)) {
          acceptStates.insert(stateMatch);
        } else {
          reportError("Expected non-empty string after \",\" in accepting state list.");
          return std::nullopt;
        }
      }
    }

    if (!match_token(RSQUARE)) {
      reportError("Expected ] at end of accepting state specification.");
      return std::nullopt;
    }

    return std::make_optional(acceptStates);
  }

  std::optional<mc::auto_map<std::string, Buchi::TransitionSet>> match_buchi_transitions() {
    mc::auto_map<std::string, Buchi::TransitionSet> transitions;

    while (match_token(LPAREN)) {
      std::string fromState;
      if (!match_token(fromState, STR)) {
        reportError("Expected non-empty string at start of transition.");
        return std::nullopt;
      }

      if (!match_token(COMMA)) {
        reportError("Expected , after start of transition.");
        return std::nullopt;
      }

      std::string label;
      if (!match_token(label, STR)) {
        reportError("Expected non-empty string for a transition label.");
        return std::nullopt;
      }

      if (!match_token(COMMA)) {
        reportError("Expected , after label of transition.");
        return std::nullopt;
      }

      std::string toState;
      if (!match_token(toState, STR)) {
        reportError("Expected non-empty string at end of transition.");
        return std::nullopt;
      }

      if (!match_token(RPAREN)) {
        reportError("Expected ) at end of transition.");
        return std::nullopt;
      }

      transitions[fromState].insert(std::make_pair(label, toState));
    }

    return transitions;
  }

  std::optional<Buchi> match_buchi() {
    if (!match_token(LBRACE)) {
      if (currentLine != "" || stream.good()) {
        reportError("Expected { at the start of a Buchi specification but did not find one.");
      }
      return std::nullopt;
    }

    auto initStates = match_buchi_init_states();
    if (!initStates) {
      return std::nullopt;
    }

    auto acceptStates = match_buchi_accept_states();
    if (!acceptStates) {
      return std::nullopt;
    }

    std::optional<mc::auto_map<std::string, Buchi::TransitionSet>> transitions = match_buchi_transitions();
    if (!transitions) {
      return std::nullopt;
    }

    if (!match_token(RBRACE)) {
      reportError("Expected } at end of Buchi specification.");
      return std::nullopt;
    }

    return std::make_optional(Buchi(*initStates,
                                    [t = *transitions](std::string const& s) {
                                      auto iter = t.find(s);
                                      return iter != t.end() ? iter->second : Buchi::TransitionSet();
                                    },
                                    [a = std::move(*acceptStates)](std::string const& s) {
                                      return a.count(s) == 1;
                                    }));
  }

  bool match_token(std::string tokenPattern) {
    std::string result;
    return match_token(result, tokenPattern);
  }

  bool match_token(std::string& result, std::string tokenPattern) {
    eat_whitespace();
    std::regex tokenPatternAtStart ("^"+tokenPattern);
    std::smatch match;
    if (std::regex_search(currentLine, match, tokenPatternAtStart)) {
      result = match[0].str();
      bool b = eat_characters(result.size());
      return b;
    }
    return false;
  }

  void eat_whitespace() {
    std::regex whitespacePattern(R"(^\s+)");
    std::smatch whitespaceMatch;

    while(std::regex_search(currentLine, whitespaceMatch, whitespacePattern) || (currentLine == "" && stream)) {
      if (!eat_characters(whitespaceMatch[0].str().size()) && !stream.eof()) {
        throw std::runtime_error ("I/O failure.");
      }
    }
  }

  bool eat_characters(int numChars) {
    while ((currentLine == "" || numChars > 0) && stream) {
      if (numChars < currentLine.size()) {
        currentLine = currentLine.substr(numChars);
        numChars = 0;
      } else {
        numChars -= currentLine.size();
        currentLine = "";
        while (std::getline(stream, currentLine) && currentLine == "") {currentLineNum++;}
        currentLineNum++; // One additional increment to account for last execution of getline in which loop condition failed.
      }
    }
    return numChars == 0;
  }

  void reportError(std::string errorMsg) {
    parseSuccessful = false;
    std::cout << "Line " << currentLineNum << ": " << errorMsg << "\n";
    if (stream.eof()) {
      std::cout << "Reached end of file before finished parsing\n";
    } else if (stream.bad()){
      std::cout << "I/O error occurred while parsing\n";
    }
  }

  bool parseSuccessful;
  int currentLineNum;
  std::string currentLine;
  std::ifstream stream;
};

int main(int argc, char* argv[]) {
  if (argc != 2) {
    std::cout << "Expected input: <buchi_filename>\n";
    return -1;
  }

  BuchiParser parser (argv[1]);
  if (!parser.ParseSuccessful()) {
    return -1;
  }

  using StringBuchi = mc::Buchi<std::string,std::string>;
  std::optional<std::pair<StringBuchi,StringBuchi>> opt_buchis;
  try {
    opt_buchis = parser.Parse();
  } catch(std::exception e) {
    std::cout << "Fatal error occurred while parsing: " << e.what() << "\n";
  }
  if (!opt_buchis) {
    return -1;
  }
  auto [buchi1, buchi2] = *opt_buchis;
  auto buchiIntersection = mc::Intersection(buchi1, buchi2);

  auto opt_lasso = mc::FindAcceptingRun(buchiIntersection);
  if (opt_lasso) {
    std::cout << "Intersection is not empty. Lasso:\n";
    const auto& [stem, loop] = *opt_lasso;
    std::cout << "Stem:\n";
    for (auto [s1, s2, x] : stem) {
      std::cout << "(" << s1 << ", " << s2 << ", " << x << ")\n";
    }
    std::cout << "\nLoop:\n";
    for (auto [s1, s2, x] : loop) {
      std::cout << "(" << s1 << ", " << s2 << ", " << x << ")\n";
    }
  } else {
    std::cout << "Intersection of Buchis is empty.\n";
  }

  return 0;
}
