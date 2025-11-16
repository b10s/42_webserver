#include "ConfigParser.hpp"

void ConfigParser::ParseMethods(Location* location) {
  std::string token;

  while (true) {
    token = Tokenize(content);
    if (token == ";") break;
    if (token == "GET")
      location->AddMethod(kGet);
    else if (token == "POST")
      location->AddMethod(kPost);
    else if (token == "DELETE")
      location->AddMethod(kDelete);
    else
      throw std::runtime_error("Invalid method in allow_methods: " + token);
  }
}
