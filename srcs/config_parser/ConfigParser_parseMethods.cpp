#include "ConfigParser.hpp"
#include "lib/http/Method.hpp"

void ConfigParser::ParseMethods(Location* location) {
  if (location->HasAllowedMethods()) {
    throw std::runtime_error("Duplicate allowed_methods directive");
  }
  std::string token;
  bool is_method_empty = true;

  while (true) {
    token = Tokenize(content);
    if (token == ";") break;
    if (token == "GET")
      location->AddAllowedMethod(lib::http::kGet);
    else if (token == "POST")
      location->AddAllowedMethod(lib::http::kPost);
    else if (token == "DELETE")
      location->AddAllowedMethod(lib::http::kDelete);
    else
      throw std::runtime_error("Invalid method in allowed_methods: " + token);
    is_method_empty = false;
  }
  if (is_method_empty) {
    throw std::runtime_error(
        "No methods specified in allowed_methods directive");
  }
  location->SetHasAllowedMethods(true);
}

// has_allowed_methods_  is set to true when there is at least one method
// directive parsed is_method_empty  is used to check if any method was added
// before encountering the semicolon
