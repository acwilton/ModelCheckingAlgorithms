#ifndef BASE_PARSER_HH
#define BASE_PARSER_HH

#include <stdexcept>
#include <iostream>
#include <istream>
#include <regex>
#include <string>

class Parser {
public:
  Parser (std::istream& stream)
    : stream(stream),
      errorsOccurred(false),
      currentLine(""),
      currentLineNum(0)
    {
      if (!stream.good()) {
        errorsOccurred = true;
        std::cout << "Parser constructed with a bad stream.\n";
      }
    }
  virtual ~Parser() {}

  virtual bool match_token(std::string tokenPattern) {
    std::string result;
    return match_token(result, tokenPattern);
  }

  virtual bool match_token(std::string& result, std::string tokenPattern) {
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

  virtual void eat_whitespace() {
    std::regex whitespacePattern(R"(^\s+)");
    std::smatch whitespaceMatch;

    while(std::regex_search(currentLine, whitespaceMatch, whitespacePattern) || (currentLine == "" && stream)) {
      if (!eat_characters(whitespaceMatch[0].str().size()) && !stream.eof()) {
        errorsOccurred = true;
        throw std::runtime_error ("I/O failure.");
      }
    }
  }

  virtual bool eat_characters(int numChars) {
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

  virtual void reportError(std::string errorMsg) {
    errorsOccurred = true;
    std::cout << "Line " << currentLineNum << ": " << errorMsg << "\n";
    if (stream.eof()) {
      std::cout << "Reached end of file before finished parsing\n";
    } else if (stream.bad()){
      std::cout << "I/O error occurred while parsing\n";
    }
  }

  virtual void transferStream(Parser* parser) {
    if (parser != nullptr) {
      int streamOffset = -static_cast<int>(currentLine.size());
      stream.unget();
      char p = stream.get();
      if (p == '\n') {
        --streamOffset; // Must decrease offset by one because currentLine doesn't include the \n that exists in the stream
      }
      stream.seekg(streamOffset, std::ios_base::cur);
      currentLine = "";
      parser->currentLineNum = currentLineNum;
    }
  }

  virtual std::string getCurrentLine() const {
    return currentLine;
  }

protected:
  std::istream& stream;
  bool errorsOccurred;
  int currentLineNum;
  std::string currentLine;
  
private:
  Parser() = delete;
  Parser(Parser const&) = delete;
  Parser(Parser&&) = delete;
  Parser& operator=(Parser const&) = delete;
  Parser& operator=(Parser&&) = delete;
};

#endif
