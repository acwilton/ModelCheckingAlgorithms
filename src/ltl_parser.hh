#ifndef LTL_PARSER_HH
#define LTL_PARSER_HH

#include <istream>
#include <string>
#include <functional>
#include <utility>
#include <optional>
#include <tuple>

#include "parser.hh"
#include "ltl.hh"

namespace parser {
  template <typename AP>
  class LTLParser {
  public:
    using Formula = mc::ltl::Formula<AP>;

    LTLParser() = default;
    LTLParser(Parser<Formula> const& apParser)
      : apParser(apParser)
      {}
    LTLParser(LTLParser const&) = default;
    LTLParser(LTLParser&&) = default;
    ~LTLParser() = default;
    LTLParser& operator=(LTLParser const&) = default;
    LTLParser& operator=(LTLParser&&) = default;

    void setAPParser(Parser<Formula> const& newAPParser) {
      apParser = newAPParser;
    }

    std::optional<Formula> operator()(ParserStream& pStream) const {
      if (!pStream.match_token(SPEC)) {
        pStream.reportError("Expected \"spec\" keyword at start of LTL specification.");
        return std::nullopt;
      }
      if (!pStream.match_token(DEF)) {
        pStream.reportError("Expected = after \"spec\".");
        return std::nullopt;
      }
      auto opt_formula = match_formula(pStream);
      if (!opt_formula) {
        pStream.reportError("Unable to parse LTL formula.");
        return opt_formula;
      }
      pStream.eat_whitespace();
      if (!pStream.errors()) {
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

    static constexpr auto SPEC = R"(spec)";
    static constexpr auto DEF = R"(=)";

    std::optional<Formula> match_formula(ParserStream& pStream) const {
      if (!pStream.match_token(LPAREN)) {
        return std::nullopt;
      }

      auto errMsgGen = [](std::string name) {
        return [name](int x) {
          std::string posString = (x == 0) ? "first" : "second";
          return "Failed to parse "+posString+" subformula of "+name+".";
        };
      };

      std::optional<Formula> opt_formula;
      if (pStream.match_token(GLOBAL)) {
        opt_formula = parseFormula<1>(pStream, mc::ltl::make_global<AP>, errMsgGen(GLOBAL));
      } else if (pStream.match_token(FUTURE)) {
        opt_formula = parseFormula<1>(pStream, mc::ltl::make_future<AP>, errMsgGen(FUTURE));
      } else if (pStream.match_token(UNTIL)) {
        opt_formula = parseFormula<2>(pStream, mc::ltl::make_until<AP>, errMsgGen(UNTIL));
      } else if (pStream.match_token(RELEASE)) {
        opt_formula = parseFormula<2>(pStream, mc::ltl::make_release<AP>, errMsgGen(RELEASE));
      } else if (pStream.match_token(NOT)) {
        opt_formula = parseFormula<1>(pStream, mc::ltl::make_not<AP>, errMsgGen(NOT));
      } else if (pStream.match_token(OR)) {
        opt_formula = parseFormula<2>(pStream, mc::ltl::make_or<AP>, errMsgGen("||"));
      } else if (pStream.match_token(AND)) {
        opt_formula = parseFormula<2>(pStream, mc::ltl::make_and<AP>, errMsgGen(AND));
      } else {
        opt_formula = apParser(pStream);
      }

      if (!pStream.match_token(RPAREN)) {
        pStream.reportError("Expected ) after formula.");
        return std::nullopt;
      }

      return opt_formula;
    }

    template <int Arity, typename FactoryType>
    auto parseFormula (ParserStream& pStream, FactoryType formulaFactory, std::function<std::string(int)>const& errMsg) const {
      auto opt_subformulas = parseN<Arity,Formula>(std::bind(&LTLParser::match_formula, *this, std::placeholders::_1), pStream, errMsg);
      return !opt_subformulas
        ? std::nullopt
        : std::make_optional(std::apply(formulaFactory, *opt_subformulas));
    };

    Parser<Formula> apParser;
  };
}

#endif
