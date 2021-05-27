#ifndef LANG_PARSER_HH
#define LANG_PARSER_HH

#include <iostream>
#include <fstream>
#include <string>
#include <utility>

#include "auto_set.hh"
#include "auto_map.hh"
#include "eq_function.hh"

#include "ltl.hh"

namespace mc {
  namespace lang {
    class LangParser {
    public:
      using State = ProgramState; // TODO: Define ProgramState
      using AP = EqFunction<bool(State const&)>;
      using Buchi = mc::Buchi<std::string, std::string>;

      LangParser(const char* filename)
        : stream(),
          currentLine(""),
          currentLineNum(0),
          parseCompleted(false),
          errorOccurred(false)
        {
          stream.open(filename);
          if (!stream.good()) {
            errorOccurred = true;
            std::cout << "Failed to open file \"" << filename << "\".\n";
            stream.close();
          }
        }
      ~LangParser() {
        if (stream.is_open()) {
          stream.close();
        }
      }

      LangParser() = delete;
      LangParser(LangParser const&) = delete;
      LangParser(LangParser&&) = delete;
      LangParser& operator=(LangParser const&) = delete;
      LangParser& operator=(LangParser&&) = delete;

      bool ParseCompleted() {
        return parseCompleted;
      }

      void Parse() {
        if (stream.is_open()) {
          // Parse LTL specification
          if (!match_token(SPEC)) {
            reportError("Must start file with specification using the \"spec\" keyword.");
            return std::nullopt;
          }
          if (!match_token(ASSIGN)) {
            reportError("Must follow spec with = sign.");
            return std::nullopt;
          }
          if (auto opt_formula = match_formula(); opt_formula) {
            // Do stuff
          } else {
            reportError("Unable to parse LTL formula.");
            return std::nullopt;
          }

          // Parse processes
          bool procsExist = true;
          while (procsExist) {
            auto opt_process = match_process();
            if (opt_process) {
              // Do stuff
            } else {
              procsExist = false;
            }
          }
          eat_whitespace();
          if (currentLine != "") {
            std::cout << "Parsing has finished but there are still contents in the file which will be ignored.\n";
          }
          stream.close();
          if (!errorOccurred) {
            parseCompleted = true;
          }
        }
      }
    private:
      // Tokens
      static constexpr auto LPAREN = R"(\()";
      static constexpr auto RPAREN = R"(\))";
      static constexpr auto LBRACE = R"(\{)";
      static constexpr auto RBRACE = R"(\})";
      static constexpr auto LSQUARE = R"(\[)";
      static constexpr auto RSQUARE = R"(\])";
    
      static constexpr auto SEMI = R"(;)";
      static constexpr auto ARROW = R"(->)";
      static constexpr auto ASSIGN = R"(=)";
    
      static constexpr auto EQUALS = R"(==)";
      static constexpr auto NOT_EQUALS = R"(!=)";
      static constexpr auto LESSER = R"(<)";
      static constexpr auto GREATER = R"(>)";
      static constexpr auto LESS_EQ = R"(<=)";
      static constexpr auto GREAT_EQ = R"(>=)";

      static constexpr auto PLUS = R"(+)";
      static constexpr auto MINUS = R"(-)";

      static constexpr auto NUM = R"(-?[1-9][0-9]*)";
      static constexpr auto NAME = R"(\w[\w\d]*)";

      static constexpr auto LOCAL = "L" + NUM;
      static constexpr auto SHARED = "S" + NUM;
      static constexpr auto INPUT = "I" + NUM;
      static constexpr auto OUTPUT = "O";

      static constexpr auto SPEC = R"(spec)";
      static constexpr auto DOT = R"(\.)";
      static constexpr auto AND = R"(&&)";
      static constexpr auto OR = R"(||)";
      static constexpr auto NOT = R"(!)";
      static constexpr auto UNTIL = "U";
      static constexpr auto RELEASE = "R";
      static constexpr auto FUTURE = "F";
      static constexpr auto GLOBAL = "G";

      static constexpr auto PROC = R"(proc)";
      static constexpr auto IF = R"(IF)";
      static constexpr auto GOTO = R"(GOTO)";

      std::optional<ltl::Formula<Program>> match_formula() {
        if (!match_token(LPAREN)) {
          return std::nullopt;
        }

        // The following two lambdas are introduced just to reduce code a bit
        auto parse1AryFormula = [&](auto formulaFactory, std::string formulaName)
          -> std::optional<ltl::Formula<State>> {
          if (auto opt_sub = match_formula(); opt_sub) {
            return std::make_optional(formulaFactory(*opt_sub));
          }
          reportError("Failed to parse subformula of " + formulaName);
          return std::nullopt;
        };
        auto parse2AryFormula = [&](auto formulaFactory, std::string formulaName)
          -> std::optional<ltl::Formula<State>> {
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
          return std::make_optional(formulaFactor(*opt_sub1, *opt_sub2));
        };

        std::optional<Formula<State>> opt_formula;
        if (match_token(GLOBAL)) {
          opt_formula = parse1AryFormula(ltl::make_global<State,AP>, GLOBAL);
        } else if (match_token(FUTURE)) {
          opt_formula = parse1AryFormula(ltl::make_future<State,AP>, FUTURE);
        } else if (match_token(UNTIL)) {
          opt_formula = parse2AryFormula(ltl::make_until<State,AP>, UNTIL);
        } else if (match_token(RELEASE)) {
          opt_formula = parse2AryFormula(ltl::make_release<State,AP>, RELEASE);
        } else if (match_token(NOT)) {
          opt_formula = parse1AryFormula(ltl::make_not<State,AP>, NOT);
        } else if (match_token(OR)) {
          opt_formula = parse2AryFormula(ltl::make_or<State,AP>, OR);
        } else if (match_token(AND)) {
          opt_formula = parse2AryFormula(ltl::make_and<State,AP>, AND);
        } else {
          // If none of the other tokens passed then this must be an atomic proposition
        }

        if (!match_token(RPAREN)) {
          reportError("Expected ) after formula.");
          return std::nullopt;
        }

        return opt_formula;
      }

      void match_process() {
        if (!match_token(PROC)) {
          if (currentLine != "" || stream.good()) {
            reportError("Expected \"proc\" keyword at the start of a process specification but did not find one.");
          }
          return std::nullopt;
        }

        std::string procName;
        if (!match_token(procName, NAME)) {
          reportError("Could not find name of process.");
          return std::nullopt;
        }

        int numOfProcesses = 1;
        if (match_token(LSQUARE)) {
          auto opt_numProc = match_number();
          if (opt_numProc.value_or(-1) < 1) {
            reportError("Expected a positive number inside of [] after process name.");
            return std::nullopt;
          }
          numOfProcesses = *opt_numProc;
        
          if(!match_token(RSQUARE)) {
            reportError("Expected a ] after specifying the number of process instances");
            return std::nullopt;
          }
        }

        if (!match_token(LBRACE)) {
          reportError("Expected a { before specifying a process's statements.");
          return std::nullopt;
        }

        auto opt_statements = match_proc_statements();

        if (!match_token(RBRACE)) {
          reportError("Expected a } after specifying a process's statements.");
          return std::nullopt;
        }
      }

      void match_proc_statements() {
        bool statementsExist = true;
        while (statementsExist) {
          if (auto opt_statement = match_proc_statement(); opt_statement) {
            // Do stuff
          } else {
            statementsExist = false;
          }
        }
      }

      void match_proc_statement() {
        std::string label;
        if (match_token(LSQUARE)) {
          if (!match_token(label, NAME)) {
            reportError("Expected a label inside of [].");
            return std::nullopt;
          }
          if (!match_token(RSQUARE)) {
            reportError("Expected a ] after a label.");
            return std::nullopt;
          }
        }

        // Check for guard
        if (auto opt_guard = match_conditional(); opt_guard) {
          // Do stuff with guard

          if (!match_token(ARROW)) {
            reportError("Expected -> after guard expression.");
            return std::nullopt;
          }
        }

        // Figure out which type of statement this is
        if (match_token(IF)) {
          // Conditional jump
          auto opt_condition = match_conditional();
          if (!opt_condition) {
            reportError("Expected conditional after IF.");
            return std::nullopt;
          }

          if (!match_token(GOTO)) {
            reportError("Expected GOTO after IF condition.");
            return std::nullopt;
          }

          std::string jumpLabel;
          if (!match_token(jumpLabel, NAME)) {
            reportError("Expected label after GOTO.");
            return std::nullopt;
          }
        
        } else if (match_token(GOTO)) {
          // Unconditional jump
          std::string jumpLabel;
          if (!match_token(jumpLabel, NAME)) {
            reportError("Expected label after GOTO.");
            return std::nullopt;
          }
        } else {
          // Assignment
          auto opt_lhs = match_variable();
          if (!opt_lhs) {
            reportError("Statement is invalid. Must be either a conditional jump, unconditional jump, or assignment.");
            return std::nullopt;
          }

          if (!match_token(ASSIGN)) {
            reportError("Expected = in an assignment statement.");
            return std::nullopt;
          }

          std::string rhs;
          auto opt_rhs_value1 = match_value();
          if (!opt_rhs_value1) {
            reportError("Expected variable or number on the right-hand side of an assignment statement.");
            return std::nullopt;
          }
          if (match_token(PLUS)) {
            auto opt_rhs_value2 = match_value();
            if (!opt_rhs_value2) {
              reportError("Expected variable or number after + on the right-hand side of an assignment statement.");
              return std::nullopt;
            }
          }
          if (match_token(MINUS)) {
            auto opt_rhs_value2 = match_value();
            if (!opt_rhs_value2) {
              reportError("Expected variable or number after + on the right-hand side of an assignment statement.");
              return std::nullopt;
            }
          }
        }
        if (!match_token(SEMI)) {
          reportError("Expected ; after statement.");
          return std::nullopt;
        }
      }

      void match_conditional() {
        if (!match_token(LPAREN)) {
          return std::nullopt;
        }
      
        auto opt_lhs = match_value();
        if (!opt_lhs) {
          reportError("Unrecognized value at left-hand side of conditional. Expect a variable or number.");
          return std::nullopt;
        }

        if (match_token(EQUALS)) {
        
        } else if (match_token(NOT_EQUALS)) {

        } else if (match_token(LESSER)) {

        } else if (match_token(LESS_EQ)) {

        } else if (match_token(GREATER)) {

        } else if (match_token(GREAT_EQ)) {

        } else {
          reportError("Expected one of ==, !=, <, <=, >, or >= between values of conditional.");
          return std::nullopt;
        }

        auto opt_rhs = match_value();
        if (!opt_rhs) {
          reportError("Unrecognized value at right-hand side of conditional. Expect a variable or number.");
          return std::nullopt;
        }

        if (!match_token(RPAREN)) {
          reportError("Expected a ) at end of conditional.");
          return std::nullopt;
        }
      }
    
      void match_value() {
        if (auto opt_var = match_variable(); opt_var) {

        } else if (auto opt_num = match_number(); opt_num) {
        
        } else {
          return std::nullopt;
        }
      }

      void match_variable() {
        std::string varStr;
        if (match_token(varStr, INPUT)) {
        
        } else if (match_token(varStr, OUTPUT)) {

        } else if (match_token(varStr, LOCAL)) {

        } else if (match_token(varStr, SHARED)) {

        } else {
          return std::nullopt;
        }
      }

      std::optional<int> match_number() {
        std::string numStr;
        return match_token(numStr, NUM)
          ? std::make_optional(std::stoi(numStr))
          : std::nullopt;
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
        errorOccurred = false;
        std::cout << "Line " << currentLineNum << ": " << errorMsg << "\n";
        if (stream.eof()) {
          std::cout << "Reached end of file before finished parsing\n";
        } else if (stream.bad()){
          std::cout << "I/O error occurred while parsing\n";
        }
      }

      bool parseCompleted;
      int currentLineNum;
      std::string currentLine;
      std::ifstream stream;
    };
  }
}

#endif
