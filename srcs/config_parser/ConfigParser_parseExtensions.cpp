#include "ConfigParser.hpp"

void ConfigParser::ParseExtensions(Location* location) {
  std::string token;

  token = Tokenize(content);
  location->SetExtension(token);
  // we are not doing bonus so only one extension is allowed here
  ConsumeExpectedSemicolon("extensions");
}

// I think I should handle the case where multiple extensions are given
// for example:
// extensions .php;
// extensions .py;
// I will handle this later
