#include "ConfigParser.hpp"
#include "lib/http/CharValidation.hpp"

void ConfigParser::ParseUploadPath(Location* location) {
  std::string token = Tokenize(content);
  if (token.empty()) {
    throw std::runtime_error(
        "Syntax error: expected upload path but got empty token");
  }
  RequireAbsoluteSafePathOrThrow(token, "Upload path");
  location->SetUploadPath(token);
  ConsumeExpectedSemicolon("upload path");
}
