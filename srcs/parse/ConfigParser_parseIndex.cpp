#include "ConfigParser.hpp"

void ConfigParser::ParseIndex(Location* location) {
  if (location->HasIndexDirective()) {
    throw std::runtime_error("Duplicate index directive");
  }
  std::string token = Tokenize(content);
  if (token.empty()) {
    throw std::runtime_error("Syntax error : expected index file name" + token);
  }
  while (token != ";") {
    location->AddIndex(token);
    token = Tokenize(content);
    if (token.empty() || IsDirective(token)) {
      throw std::runtime_error(
          "Syntax error : expected ';' after index file names" + token);
    }
  }
  location->SetHasIndexDirective(true);
}
