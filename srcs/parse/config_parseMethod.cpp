#include "config_parser.hpp"

void ConfigParser::parseMethods(Location *location) {
  std::string token;

  while (true) {
    token = tokenize(content_);
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
