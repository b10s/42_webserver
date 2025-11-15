#include "ConfigParser.hpp"

void ConfigParser::consumeMethods(Location* location) {
  std::string token;

  while (true) {
    token = Tokenize(content_);
    if (token == ";") break;
    if (token == "GET")
      location->addMethod(GET);
    else if (token == "POST")
      location->addMethod(POST);
    else if (token == "DELETE")
      location->addMethod(DELETE);
    else
      throw std::runtime_error("Invalid method in allow_methods: " + token);
  }
}
