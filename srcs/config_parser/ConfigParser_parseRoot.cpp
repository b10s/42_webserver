#include "ConfigParser.hpp"
#include "lib/http/CharValidation.hpp"

void ConfigParser::ParseRoot(Location* location) {
  std::string token = Tokenize(content);
  if (token.empty()) {
    throw std::runtime_error(
        "Syntax error: expected root path but got empty token");
  }
  RequireAbsoluteSafePathOrThrow(token, "Root path");
  location->SetRoot(token);
  ConsumeExpectedSemicolon("root path");
}
