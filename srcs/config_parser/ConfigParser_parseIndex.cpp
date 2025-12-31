#include "ConfigParser.hpp"
#include "lib/http/CharValidation.hpp"

bool ConfigParser::IsSafeIndexFilename(const std::string& filename) const {
  if (filename.empty()) return false;
  if (filename.find("../") != std::string::npos) return false;
  if (filename.find('/') != std::string::npos)
    return false;  // absolute path or subdir
  for (size_t i = 0; i < filename.size(); ++i) {
    unsigned char c = static_cast<unsigned char>(filename[i]);
    if (!lib::http::IsVisibleAscii(c)) return false;  // non-visible ASCII
    // reject unsafe chars such as spaces, quotes, angle brackets, etc.
    if (!(std::isalnum(c) || c == '.' || c == '-' || c == '_')) return false;
  }
  return true;
}

void ConfigParser::ParseIndex(Location* location) {
  std::string token = Tokenize(content);
  if (token.empty()) {
    throw std::runtime_error(
        "Syntax error: expected index file name  but got empty token");
  }
  if (!IsSafeIndexFilename(token)) {
    throw std::runtime_error("Unsafe index file name: " + token);
  }
  location->SetIndexFile(token);
  ConsumeExpectedSemicolon("index file name");
}
