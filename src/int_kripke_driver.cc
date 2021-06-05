#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

#include "auto_set.hh"
#include "eq_function.hh"

#include "base_parser.hh"
#include "ltl_parser.hh"

#include "kripke.hh"
#include "ltl.hh"
#include "model_check.hh"

using namespace mc;

using AP = EqFunction<bool(int const&)>;
using Formula = ltl::Formula<AP>;

/*
class IntKripkeParser final : public Parser{

};

class ArithmeticParser final : public Parser {

};
*/

class CollatzAPParser final : public Parser<ltl::Formula<AP>>{
public:
  using ExprType = std::function<int(int)>;

  using Parser<Formula>::reportError;
  using Parser<Formula>::match_token;

  CollatzAPParser(std::istream* stream, int N)
    : Parser<Formula>(stream),
      N(N)
    {}
  ~CollatzAPParser() = default;

  std::optional<Formula> parse() {
    if (match_token(EQUALS)) {
      auto opt_operands = match_operands("==");
      return !opt_operands
        ? std::nullopt
        : std::make_optional(ltl::make_atomic<AP>([opt_operands](int x) {
          auto& [l,r] = *opt_operands;
          return l(x) == r(x);
        }));
    } else if (match_token(NOT_EQUALS)) {
      auto opt_operands = match_operands("!=");
      return !opt_operands
        ? std::nullopt
        : std::make_optional(ltl::make_atomic<AP>([opt_operands](int x) {
          auto& [l,r] = *opt_operands;
          return l(x) != r(x);
        }));
    } else if (match_token(LESS_EQ)) {
      auto opt_operands = match_operands("<=");
      return !opt_operands
        ? std::nullopt
        : std::make_optional(ltl::make_atomic<AP>([opt_operands](int x) {
          auto& [l,r] = *opt_operands;
          return l(x) <= r(x);
        }));
    } else if (match_token(LESSER)) {
      auto opt_operands = match_operands("<");
      return !opt_operands
        ? std::nullopt
        : std::make_optional(ltl::make_atomic<AP>([opt_operands](int x) {
          auto& [l,r] = *opt_operands;
          return l(x) < r(x);
        }));
    } else if (match_token(GREAT_EQ)) {
      auto opt_operands = match_operands(">=");
      return !opt_operands
        ? std::nullopt
        : std::make_optional(ltl::make_atomic<AP>([opt_operands](int x) {
          auto& [l,r] = *opt_operands;
          return l(x) >= r(x);
        }));
    } else if (match_token(GREATER)) {
      auto opt_operands = match_operands(">");
      return !opt_operands
        ? std::nullopt
        : std::make_optional(ltl::make_atomic<AP>([opt_operands](int x) {
          auto& [l,r] = *opt_operands;
          return l(x) > r(x);
        }));
    } else {
      reportError("Expected one of ==, !=, <, <=, >, or >= at start of atomic proposition.");
      return std::nullopt;
    }
  }
      
private:
  static constexpr auto LPAREN = R"(\()";
  static constexpr auto RPAREN = R"(\))";

  static constexpr auto EQUALS = R"(==)";
  static constexpr auto NOT_EQUALS = R"(=/=)";
  static constexpr auto LESSER = R"(<)";
  static constexpr auto GREATER = R"(>)";
  static constexpr auto LESS_EQ = R"(<=)";
  static constexpr auto GREAT_EQ = R"(>=)";

  static constexpr auto PLUS = R"(\+)";
  static constexpr auto MINUS = R"(-)";
  static constexpr auto TIMES = R"(\*)";
  static constexpr auto DIV = R"(/)";
  static constexpr auto MOD = R"(\%)";

  static constexpr auto NUM = R"(-?[1-9][0-9]*|0)";
  static constexpr auto VAR = R"(x)";
  static constexpr auto CAP = R"(N)";

  std::optional<ExprType> match_expression() {
    std::optional<ExprType> retVal;
    if (match_token(VAR)) {
      return std::make_optional([](int x) { return x; });
    } else if(match_token(CAP)) {
      return std::make_optional([N=this->N](int) { return N; });
    } else if (auto opt_num = match_number(); opt_num) {
      return std::make_optional([retNum = *opt_num](int) { return retNum; });
    } else if (match_token(LPAREN)) {
      if (match_token(PLUS)) {
        auto opt_operands = match_operands("+");
        retVal =  !opt_operands
          ? std::nullopt
          : std::make_optional([opt_operands](int x) {
            auto& [l,r] = *opt_operands;
            return l(x) + r(x);
          });
      } else if (match_token(MINUS)) {
        auto opt_operands = match_operands("-");
        retVal =  !opt_operands
          ? std::nullopt
          : std::make_optional([opt_operands](int x) {
            auto& [l,r] = *opt_operands;
            return l(x) - r(x);
          });
      } else if (match_token(TIMES)) {
        auto opt_operands = match_operands("*");
        retVal =  !opt_operands
          ? std::nullopt
          : std::make_optional([opt_operands](int x) {
            auto& [l,r] = *opt_operands;
            return l(x) * r(x);
          });
      } else if (match_token(DIV)) {
        auto opt_operands = match_operands("/");
        retVal =  !opt_operands
          ? std::nullopt
          : std::make_optional([opt_operands](int x) {
            auto& [l,r] = *opt_operands;
            return l(x) / r(x);
          });
      } else if (match_token(MOD)) {
        auto opt_operands = match_operands("%");
        retVal =  !opt_operands
          ? std::nullopt
          : std::make_optional([opt_operands](int x) {
            auto& [l,r] = *opt_operands;
            return l(x) % r(x);
          });
      } else {
        reportError("Expected one of +, -, *, /, or % at the start of an arithmetic expression.");
        return std::nullopt;
      }

      if (!match_token(RPAREN)) {
        reportError("Expected ) at the end of an arithmetic expression.");
        return std::nullopt;
      }
      return retVal;
    } else {
      reportError("Expected a number, \"x\", \"N\", or ( for the start of a new expression.");
      return std::nullopt;
    }
  }

  std::optional<std::pair<ExprType,ExprType>> match_operands (std::string operationName) {
    auto opt_leftOp = match_expression();
    if (!opt_leftOp) {
      reportError("Could not parse left operand of "+operationName);
      return std::nullopt;
    }
    auto opt_rightOp = match_expression();
    if (!opt_rightOp) {
      reportError("Could not parse right operand of "+operationName);
      return std::nullopt;
    }
    return std::make_optional(std::make_pair(*opt_leftOp, *opt_rightOp));
  }

  std::optional<int> match_number() {
    std::string numStr;
    return match_token(numStr, NUM)
      ? std::make_optional(std::stoi(numStr))
      : std::nullopt;
  }

  int N;
};

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

  auto apParserPtr = std::make_shared<CollatzAPParser>(&stream,N);
  LTLParser<AP> ltlParser (&stream, apParserPtr);

  std::optional<Formula> opt_spec;
  try {
    opt_spec = ltlParser.parse();
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
