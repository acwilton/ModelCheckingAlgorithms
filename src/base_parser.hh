#ifndef BASE_PARSER_HH
#define BASE_PARSER_HH

#include <stdexcept>
#include <iostream>
#include <istream>
#include <regex>
#include <string>

template <typename R>
class Parser {
public:
  Parser ()
    : Parser(nullptr)
    {}

  Parser (std::istream* streamPtr)
    : streamPtr(streamPtr),
      errorsOccurred(false),
      currentLine(""),
      currentLineNum(0)
    {
      if (!stream().good()) {
        errorsOccurred = true;
        std::cout << "Parser constructed with a bad stream.\n";
      }
    }
  virtual ~Parser() {}

  // Call our parse function but with another parser's stream
  template <typename T>
  std::optional<R> parse(Parser<T>* other) {
    other->transferStream(this);
    auto opt_v = parse();
    transferStream(other);
    return opt_v;
  }

  virtual std::optional<R> parse() = 0;

  template <typename T>
  friend class Parser; // Parser is friends with all other Parser's

protected:
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

    while(std::regex_search(currentLine, whitespaceMatch, whitespacePattern) || (currentLine == "" && stream())) {
      if (!eat_characters(whitespaceMatch[0].str().size()) && !stream().eof()) {
        errorsOccurred = true;
        throw std::runtime_error ("I/O failure.");
      }
    }
  }

  virtual bool eat_characters(int numChars) {
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

  virtual void reportError(std::string errorMsg) {
    errorsOccurred = true;
    std::cout << "Line " << currentLineNum << ": " << errorMsg << "\n";
    if (stream().eof()) {
      std::cout << "Reached end of file before finished parsing\n";
    } else if (stream().bad()){
      std::cout << "I/O error occurred while parsing\n";
    }
  }

  template <typename T>
  void transferStream(Parser<T>* parser) {
    parser->streamPtr = streamPtr;
    parser->currentLine = currentLine;
    parser->currentLineNum = currentLineNum;
  }

  std::istream& stream() {
    static std::istream badStream(nullptr);
    return (streamPtr == nullptr) ? badStream : *streamPtr;
  }

  bool errors() {
    return errorsOccurred;
  }

private:
  std::istream* streamPtr;
  std::string currentLine;
  bool errorsOccurred;
  int currentLineNum;

  Parser(Parser const&) = delete;
  Parser(Parser&&) = delete;
  Parser& operator=(Parser const&) = delete;
  Parser& operator=(Parser&&) = delete;
};

#endif
