#ifndef LTL_PARSER_HH
#define LTL_PARSER_HH

#include <istream>
#include <string>
#include <functional>
#include <utility>
#include <optional>
#include <memory>

#include "base_parser.hh"
#include "ltl.hh"

template <typename AP>
class LTLParser final : public Parser<mc::ltl::Formula<AP>>{
public:
  using Formula = mc::ltl::Formula<AP>;
  using Parser<Formula>::reportError;
  using Parser<Formula>::match_token;
  using Parser<Formula>::eat_whitespace;
  using Parser<Formula>::errors;

  LTLParser(std::istream* stream, std::shared_ptr<Parser<Formula>> apParser)
    : Parser<Formula>(stream),
      apParser(apParser)
    {}
  ~LTLParser() = default;

  std::optional<Formula> parse() {
    auto opt_formula = match_formula();
    if (!opt_formula) {
      reportError("Unable to parse LTL formula.");
      return opt_formula;
    }
    eat_whitespace();
    if (!errors()) {
      return opt_formula;
    }
    return std::nullopt;
  }
      
private:
  // Tokens
  static constexpr auto LPAREN = R"(\()";
  static constexpr auto RPAREN = R"(\))";
    
  static constexpr auto EQUALS = R"(\=\=)";
  static constexpr auto NOT_EQUALS = R"(\!\=)";
  static constexpr auto LESSER = R"(<)";
  static constexpr auto GREATER = R"(>)";
  static constexpr auto LESS_EQ = R"(<\=)";
  static constexpr auto GREAT_EQ = R"(>\=)";

  static constexpr auto NUM = R"(-?[1-9][0-9]*)";

  static constexpr auto AND = R"(&&)";
  static constexpr auto OR = R"(\|\|)";
  static constexpr auto NOT = R"(!)";
  static constexpr auto UNTIL = "U";
  static constexpr auto RELEASE = "R";
  static constexpr auto FUTURE = "F";
  static constexpr auto GLOBAL = "G";

  std::optional<Formula> match_formula() {
    if (!match_token(LPAREN)) {
      return std::nullopt;
    }

    // The following two lambdas are introduced just to reduce code a bit
    auto parse1AryFormula = [&](auto formulaFactory, std::string formulaName)
      -> std::optional<Formula> {
      if (auto opt_sub = match_formula(); opt_sub) {
        return std::make_optional(formulaFactory(*opt_sub));
      }
      reportError("Failed to parse subformula of " + formulaName);
      return std::nullopt;
    };
    auto parse2AryFormula = [&](auto formulaFactory, std::string formulaName)
      -> std::optional<Formula> {
      auto opt_sub1 = match_formula();
      if (!opt_sub1) {
        reportError("Failed to parse first subformula of "+formulaName);
        return std::nullopt;
      }
      auto opt_sub2 = match_formula();
      if (!opt_sub2) {
        reportError("Failed to parse second subformula of "+formulaName);
        return std::nullopt;
      }
      return std::make_optional(formulaFactory(*opt_sub1, *opt_sub2));
    };

    std::optional<Formula> opt_formula;
    if (match_token(GLOBAL)) {
      opt_formula = parse1AryFormula(mc::ltl::make_global<AP>, GLOBAL);
    } else if (match_token(FUTURE)) {
      opt_formula = parse1AryFormula(mc::ltl::make_future<AP>, FUTURE);
    } else if (match_token(UNTIL)) {
      opt_formula = parse2AryFormula(mc::ltl::make_until<AP>, UNTIL);
    } else if (match_token(RELEASE)) {
      opt_formula = parse2AryFormula(mc::ltl::make_release<AP>, RELEASE);
    } else if (match_token(NOT)) {
      opt_formula = parse1AryFormula(mc::ltl::make_not<AP>, NOT);
    } else if (match_token(OR)) {
      opt_formula = parse2AryFormula(mc::ltl::make_or<AP>, "||");
    } else if (match_token(AND)) {
      opt_formula = parse2AryFormula(mc::ltl::make_and<AP>, AND);
    } else {
      opt_formula = apParser->parse(this);
    }

    if (!match_token(RPAREN)) {
      reportError("Expected ) after formula.");
      return std::nullopt;
    }

    return opt_formula;
  }

  std::shared_ptr<Parser<Formula>> apParser;
};

#endif
