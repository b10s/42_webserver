#include "ConfigParser.hpp"
#include "lib/http/CharValidation.hpp"

void ConfigParser::IsSafeRootPathOrThrow(const std::string& path) {
  if (path.empty() || path[0] != '/') {
    throw std::runtime_error("Root path must be an absolute path: " + path);
  }
  for (size_t i = 0; i < path.size(); ++i) {
    if (!lib::http::IsValidHeaderChar(path[i])) {
      throw std::runtime_error("Root path contains invalid character: " + path);
    }
  }
}

void ConfigParser::ParseRoot(Location* location) {
  std::string token = Tokenize(content);
  if (token.empty()) {
    throw std::runtime_error(
        "Syntax error: expected root path but got empty token");
  }
  IsSafeRootPathOrThrow(token);
  location->SetRoot(token);
  ConsumeExpectedSemicolon("root path");
}
