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

class CollatzAPParser final : public Parser{
public:

  CollatzAPParser(std::istream& stream)
    : Parser(stream)
    {}
  ~CollatzAPParser() = default;

  std::optional<ltl::Formula<AP>> Parse() {
    enum class ComparisonType {
      EQUALS,
      NOT_EQUALS,
      LESSER,
      LESS_EQ,
      GREATER,
      GREAT_EQ
    };

    ComparisonType comparison;
    if (match_token(EQUALS)) {
      comparison = ComparisonType::EQUALS;
    } else if (match_token(NOT_EQUALS)) {
      comparison = ComparisonType::NOT_EQUALS;
    } else if (match_token(LESSER)) {
      comparison = ComparisonType::LESSER;
    } else if (match_token(LESS_EQ)) {
      comparison = ComparisonType::LESS_EQ;
    } else if (match_token(GREATER)) {
      comparison = ComparisonType::GREATER;
    } else if (match_token(GREAT_EQ)) {
      comparison = ComparisonType::GREAT_EQ;
    } else {
      reportError("Expected one of ==, !=, <, <=, >, or >= at start of atomic proposition.");
      return std::nullopt;
    }
    
    bool leftIsInput = false;
    int leftInt;
    if (auto opt_leftInt = match_number(); opt_leftInt) {
      leftInt = *opt_leftInt;
    } else if (match_token(VAR)) {
      leftIsInput = true;
    } else {
      reportError("Unrecognized value at left-hand side of conditional. Expect a number or \"x\".");
      return std::nullopt;
    }

    bool rightIsInput = false;
    int rightInt;
    if (auto opt_rightInt = match_number(); opt_rightInt) {
      rightInt = *opt_rightInt;
    } else if (match_token(VAR)) {
      rightIsInput = true;
    } else {
      reportError("Unrecognized value at right-hand side of conditional. Expect a number or \"x\".");
      return std::nullopt;
    }

    if (!errorsOccurred) {
      return std::make_optional(
        ltl::make_atomic<AP>(
          [comparison,leftIsInput,leftInt,rightIsInput,rightInt](int x) {
            int left = leftIsInput ? x : leftInt;
            int right = rightIsInput ? x : rightInt;
            switch(comparison) {
            case ComparisonType::EQUALS:
              return left == right;
            case ComparisonType::NOT_EQUALS:
              return left != right;
            case ComparisonType::LESSER:
              return left < right;
            case ComparisonType::LESS_EQ:
              return left <= right;
            case ComparisonType::GREATER:
              return left > right;
            case ComparisonType::GREAT_EQ:
              return left == right;
            }
          }));
    }
    return std::nullopt;
  }
      
private:
  static constexpr auto EQUALS = R"(==)";
  static constexpr auto NOT_EQUALS = R"(!=)";
  static constexpr auto LESSER = R"(<)";
  static constexpr auto GREATER = R"(>)";
  static constexpr auto LESS_EQ = R"(<=)";
  static constexpr auto GREAT_EQ = R"(>=)";

  static constexpr auto NUM = R"(-?[1-9][0-9]*)";
  static constexpr auto VAR = R"(x)";

  std::optional<int> match_number() {
    std::string numStr;
    return match_token(numStr, NUM)
      ? std::make_optional(std::stoi(numStr))
      : std::nullopt;
  }
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

  LTLParser<AP> ltlParser (stream, [](std::istream& stream) {
    CollatzAPParser apParser (stream);
    return apParser.Parse();
  });

  std::optional<ltl::Formula<AP>> opt_spec;
  try {
    opt_spec = ltlParser.Parse();
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
