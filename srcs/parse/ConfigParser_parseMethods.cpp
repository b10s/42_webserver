#include "ConfigParser.hpp"

void ConfigParser::ParseMethods(Location* location) {
  if (location->HasAllowMethods()) {
    throw std::runtime_error("Duplicate allow_methods directive");
  }
  std::string token;
  bool is_method_empty = true;

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
    is_method_empty = false;
  }
  if (is_method_empty) {
    throw std::runtime_error("No methods specified in allow_methods directive");
  }
  location->SetHasAllowMethods(true);
}

// has_allow_methods_  is set to true when there is at least one method
// directive parsed is_method_empty  is used to check if any method was added
// before encountering the semicolon
