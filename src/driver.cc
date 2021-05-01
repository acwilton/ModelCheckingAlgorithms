#include <iostream>
#include <string>
#include <regex>

#include "buchi.hh"

//Buchi<std::string>
void
stringToBuchi(std::string str) {
  std::smatch buchiResult;
  std::regex buchiPattern (R"(^(\{(\([^,\r\n]+,[^,\r\n]+,[^,\r\n]+\))+\}))");
  while (std::regex_search(str, buchiResult, buchiPattern)) {
    std::cout << "Buchi: " << buchiResult[1] << "\n";
    std::string substring = buchiResult[1].str().substr(1);
    std::smatch transitionResult;
    std::regex transitionPattern (R"(^\(([^,\r\n]+),([^,\r\n]+),([^,\r\n]+)\))");
    while (std::regex_search(substring, transitionResult, transitionPattern)) {
      std::cout << transitionResult[1] << " "
                << transitionResult[2] << " "
                << transitionResult[3] << "\n";
      substring = transitionResult.suffix();
    }
    str = buchiResult.suffix();
  }
}

int main(int argc, char* argv[]) {
  stringToBuchi("{(avbc,afe,wefw)(awfe,re,32rdeaE)}{(wow,a,cow)(super,cool,man)(what,it,do)}");
}
