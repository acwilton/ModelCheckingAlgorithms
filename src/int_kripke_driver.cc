#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

#include "auto_set.hh"
#include "eq_function.hh"

#include "parser.hh"
#include "parser_utils.hh"
#include "ltl_parser.hh"

#include "kripke.hh"
#include "ltl.hh"
#include "model_check.hh"

using namespace mc;

using AP = EqFunction<bool(int const&)>;
using Formula = ltl::Formula<AP>;

namespace parser {
  class ArithmeticParser {
  public:
    using ArithType = std::function<int(int)>;

    ArithmeticParser(int N)
      : N(N)
      {}
    ArithmeticParser(ArithmeticParser&&) = default;
    ArithmeticParser(ArithmeticParser const&) = default;
    ~ArithmeticParser() = default;
    ArithmeticParser& operator=(ArithmeticParser const&) = default;
    ArithmeticParser& operator=(ArithmeticParser&&) = default;

    std::optional<ArithType> operator()(ParserStream& pStream) {
      if (pStream.match_token(VAR)) {
        return std::make_optional([](int s) { return s; });
      } else if(pStream.match_token(CAP)) {
        return std::make_optional([N=this->N](int) { return N; });
      } else if (auto opt_num = match_integer(pStream); opt_num) {
        return std::make_optional([retNum = *opt_num](int) { return retNum; });
      } else if (pStream.match_token(LPAREN)) {
        std::optional<ArithType> retVal;

        auto errMsgGen = [](std::string operationName) {
          return [operationName](int i) {
            std::string posString = (i == 0) ? "left" : "right";
            return "Failed to parse "+posString+" operand of "+operationName+".";
          };
        };

        auto parseOperation = [this,&pStream,&errMsgGen](std::function<int(int,int)>const& operation, std::string opName)
          -> std::optional<ArithType> {
          auto opt_operands = parseN<2,ArithType>(*this, pStream, errMsgGen(opName));
          return !opt_operands
          ? std::nullopt
          : std::make_optional([operation, opt_operands](int x) {
            auto& [l, r] = *opt_operands;
            return operation(l(x), r(x));
          });
        };

        if (pStream.match_token(PLUS)) {
          retVal = parseOperation(std::plus<int>{}, "+");
        } else if (pStream.match_token(MINUS)) {
          retVal = parseOperation(std::minus<int>{}, "-");
        } else if (pStream.match_token(TIMES)) {
          retVal = parseOperation(std::multiplies<int>{}, "*");
        } else if (pStream.match_token(DIV)) {
          retVal = parseOperation(std::divides<int>{}, "/");
        } else if (pStream.match_token(MOD)) {
          retVal = parseOperation(std::modulus<int>{}, "%");
        } else {
          pStream.reportError("Expected one of +, -, *, /, or % at the start of an arithmetic expression.");
          return std::nullopt;
        }

        if (!pStream.match_token(RPAREN)) {
          pStream.reportError("Expected ) at the end of an arithmetic expression.");
          return std::nullopt;
        }
        return retVal;
      } else {
        pStream.reportError("Expected a number, \"s\", \"N\", or ( for the start of a new expression.");
        return std::nullopt;
      }
    }

  private:
    static constexpr auto LPAREN = R"(\()";
    static constexpr auto RPAREN = R"(\))";

    static constexpr auto PLUS = R"(\+)";
    static constexpr auto MINUS = R"(-)";
    static constexpr auto TIMES = R"(\*)";
    static constexpr auto DIV = R"(/)";
    static constexpr auto MOD = R"(\%)";

    static constexpr auto VAR = R"(x)";
    static constexpr auto CAP = R"(N)";

    int N;
  };

  class ComparisonParser{
  public:
    using CompType = std::function<bool(int)>;

    ComparisonParser(int N)
      : arithParser(N)
      {}
    ~ComparisonParser() = default;

    std::optional<CompType> operator()(ParserStream& pStream) const {
      auto errMsgGen = [](std::string comparisonName) {
        return [comparisonName](int i) {
          std::string posString = (i == 0) ? "left" : "right";
          return "Failed to parse "+posString+" operand of "+comparisonName+".";
        };
      };

      auto parseComparison = [&arithParser=this->arithParser,&pStream,&errMsgGen](std::function<int(int,int)>const& comparison, std::string compName)
        -> std::optional<CompType> {
        auto opt_operands = parseN<2,ArithmeticParser::ArithType>(arithParser, pStream, errMsgGen(compName));
        return !opt_operands
        ? std::nullopt
        : std::make_optional([comparison, opt_operands](int x) {
          auto& [l, r] = *opt_operands;
          return comparison(l(x), r(x));
        });
      };
      if (pStream.match_token(EQUALS)) {
        return parseComparison(std::equal_to<int>{}, "==");
      } else if (pStream.match_token(NOT_EQUALS)) {
        return parseComparison(std::not_equal_to<int>{}, "=/=");
      } else if (pStream.match_token(LESS_EQ)) {
        return parseComparison(std::less_equal<int>{}, "<=");
      } else if (pStream.match_token(LESSER)) {
        return parseComparison(std::less<int>{}, "<");
      } else if (pStream.match_token(GREAT_EQ)) {
        return parseComparison(std::greater_equal<int>{}, ">=");
      } else if (pStream.match_token(GREATER)) {
        return parseComparison(std::greater<int>{}, ">");
      } else {
        pStream.reportError("Expected one of ==, !=, <, <=, >, or >=.");
        return std::nullopt;
      }
    }
  private:
    static constexpr auto EQUALS = R"(==)";
    static constexpr auto NOT_EQUALS = R"(=/=)";
    static constexpr auto LESSER = R"(<)";
    static constexpr auto GREATER = R"(>)";
    static constexpr auto LESS_EQ = R"(<=)";
    static constexpr auto GREAT_EQ = R"(>=)";

    ArithmeticParser arithParser;
  };

  class IntAPParser {
  public:
    IntAPParser(int N)
      : compParser(N)
      {}
    ~IntAPParser() = default;

    std::optional<Formula> operator()(ParserStream& pStream) {
      auto opt_func = compParser(pStream);
      return !opt_func
        ? std::nullopt
        : std::make_optional(ltl::make_atomic<AP>(*opt_func));
    }

  private:
    ComparisonParser compParser;
  };
}

void PrintUsage() {
  std::cout << "Usage: collatz <ltl_filename> [modulo_int]\n";
  std::cout << "This will read the ltl specification provided in the ltl_filename and model check it on the reverse collatz graph modulo the modula_int parameter provided.\n";
  std::cout << "modulo_int must be greater than 0 and if it is not provided, it will default to the arbitrary number 1170.\n";
}

int main(int argc, char* argv[]) {
  int N = 1170; // Arbitrary number
  if (argc > 3) {
    std::cout << "Too many arguments. Expected at most 2 but got " << argc - 1 << "\n\n";
    PrintUsage();
    return -1;
  }
  if (argc == 1) {
    PrintUsage();
    return -1;
  }
  
  if (argc == 3) {
    try {
      N = std::stoi(argv[2]);
    } catch (std::exception e) {
      std::cout << "Could not parse argument. Must be a positive integer.\n";
      return -1;
    }
    if (N < 1) {
      std::cout << "integer argument must be positive.\n";
      return -1;
    }
  }

  std::ifstream stream;
  stream.open(argv[1]);
  if (!stream.good()) {
    std::cout << "Failed to open file \"" << argv[1] << "\".\n";
    return -1;
  }

  parser::ParserStream pStream(&stream);
  parser::IntAPParser intAPParser(N);
  parser::LTLParser<AP> ltlParser (intAPParser);

  std::optional<Formula> opt_spec;
  try {
    opt_spec = ltlParser(pStream);
  } catch(std::exception e) {
    std::cout << "Fatal error occurred while parsing: " << e.what() << "\n";
    stream.close();
    return -1;
  }
  if (!opt_spec) {
    stream.close();
    return -1;
  }
  auto spec = ltl::make_not(*opt_spec); // We negate so that we properly check for existance of counterexample
  
  if (std::string dummyLine; std::getline(stream, dummyLine) && dummyLine != "") {
    std::cout << "Parsing has completed, but there is still excess content in the file. Ignoring.\n";
  }
  stream.close();

  // Defining the Collatz graph (https://en.wikipedia.org/wiki/Collatz_conjecture#In_reverse)
  // as a kripke structure except that all numbers are taken mod N (to make it finite)
  Kripke<int> reverseCollatz(
    {1}, // Start state is the number 1
    [N](int x) {
      auto_set<int> nextInts {(2*x) % N};
      if (x % 6 == 4) {
        nextInts.insert(((x - 1)/3) % N);
      }
      return nextInts;
    });

  auto opt_lasso = ModelCheck(reverseCollatz, ltl::Normalize(spec));
  if (opt_lasso) {
    std::cout << "The LTL specification does not hold.\n";
    const auto& [stem, loop] = *opt_lasso;
    std::cout << "Stem:\n";
    for (auto state : stem) {
      std::cout << state << "\n";
    }
    std::cout << "Loop:\n";
    for (auto state : loop) {
      std::cout << state << "\n";
    }
  } else {
    std::cout << "The LTL specification holds.\n";
  }
}
