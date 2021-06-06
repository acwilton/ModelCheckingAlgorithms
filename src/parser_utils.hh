#ifndef PARSER_UTILS_HH
#define PARSER_UTILS_HH

#include <optional>
#include <array>
#include <vector>

#include "parser.hh"

namespace parser {
  template <size_t N, typename R>
  std::optional<std::array<R,N>> parseN(Parser<R> const& parser, ParserStream& pStream, std::function<std::string(int)> errMsg) {
    std::array<R,N> parsedArr = {};
    for (size_t i = 0; i < N; ++i) {
      auto opt_value = parser(pStream);
      if (!opt_value) {
        pStream.reportError(errMsg(i));
        return std::nullopt;
      }
      parsedArr[i] = *opt_value;
    }
    return std::make_optional(parsedArr);
  }

  template <typename R>
  std::vector<R> parseStar(Parser<R> const& parser, ParserStream& pStream) {
    std::vector<R> parsedVec;
    while (true) {
      auto opt_value = parser(pStream);
      if (!opt_value) { return parsedVec; }
      parsedVec.emplace_back(*opt_value);
    }
  }

  std::optional<int> match_integer(ParserStream& parser) {
    std::string numStr;
    return parser.match_token(numStr, R"(-?[1-9][0-9]*|0)")
      ? std::make_optional(std::stoi(numStr))
      : std::nullopt;
  }

  template <typename R>
  class SeparatedParser {
  public:
    SeparatedParser(Parser<R> const& parser, std::string sepToken)
      : parser(parser),
        sepToken(sepToken),
        matchSep(false)
      {}
    ~SeparatedParser() = default;

    std::optional<R> operator()(ParserStream& pStream) const {
      if (matchSep) {
        if (!pStream.match_token(sepToken)) {
          return std::nullopt;
        }
      }
      auto opt_val = parser(pStream);
      if (opt_val) {
        matchSep = true;
      }
      return opt_val;
    }

    void reset() { matchSep = false; }

  private:
    mutable bool matchSep;
    std::string sepToken;
    Parser<R> parser;
  };
}

#endif
