#ifndef PARSER_STREAM_HH
#define PARSER_STREAM_HH

#include <stdexcept>
#include <iostream>
#include <istream>
#include <regex>
#include <string>
#include <functional>
#include <optional>

namespace parser {
  class ParserStream {
  public:
    ParserStream ()
      : ParserStream(nullptr)
      {}
    ParserStream (std::istream* streamPtr)
      : streamPtr(streamPtr),
        errorsOccurred(false),
        currentLine(""),
        currentLineNum(0)
      {}
    ParserStream(ParserStream const&) = default;
    ParserStream(ParserStream&&) = default;
    ParserStream& operator=(ParserStream const&) = default;
    ParserStream& operator=(ParserStream&&) = default;

    ~ParserStream() {}

    bool match_token(std::string tokenPattern) {
      std::string result;
      return match_token(result, tokenPattern);
    }

    bool match_token(std::string& result, std::string tokenPattern) {
      eat_whitespace();
      std::regex tokenPatternAtStart ("^("+tokenPattern+")");
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

      while(std::regex_search(currentLine, whitespaceMatch, whitespacePattern) || (currentLine == "" && stream())) {
        if (!eat_characters(whitespaceMatch[0].str().size()) && !stream().eof()) {
          errorsOccurred = true;
          throw std::runtime_error ("I/O failure.");
        }
      }
    }

    bool eat_characters(int numChars) {
      while ((currentLine == "" || numChars > 0) && stream()) {
        if (numChars < currentLine.size()) {
          currentLine = currentLine.substr(numChars);
          numChars = 0;
        } else {
          numChars -= currentLine.size();
          currentLine = "";
          while (std::getline(stream(), currentLine) && currentLine == "") {currentLineNum++;}
          currentLineNum++; // One additional increment to account for last execution of getline in which loop condition failed.
        }
      }
      return numChars == 0;
    }

    void reportError(std::string errorMsg) noexcept {
      errorsOccurred = true;
      std::cout << "Line " << currentLineNum << ": " << errorMsg << "\n";
      if (stream().eof()) {
        std::cout << "Reached end of file before finished parsing\n";
      } else if (stream().bad()){
        std::cout << "I/O error occurred while parsing\n";
      }
    }

    std::istream& stream() noexcept {
      static std::istream badStream(nullptr);
      return (streamPtr == nullptr) ? badStream : *streamPtr;
    }

    bool errors() const noexcept {
      return errorsOccurred;
    }

  private:
    std::istream* streamPtr;
    std::string currentLine;
    bool errorsOccurred;
    int currentLineNum;
  };

  template <typename R>
  using Parser = std::function<std::optional<R>(ParserStream&)>;
}

#endif
