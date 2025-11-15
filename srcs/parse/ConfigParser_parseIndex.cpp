#include "ConfigParser.hpp"

void ConfigParser::parseIndex(Location* location) {
  std::string token = tokenize(content_);
  if (token.empty()) {
    throw std::runtime_error("Syntax error : expected index file name" + token);
  }
  while (token != ";") {
    location->AddIndex(token);
    token = tokenize(content_);
    if (token.empty() || isDirective(token)) {
      throw std::runtime_error(
          "Syntax error : expected ';' after index file names" + token);
    }
  }
}
