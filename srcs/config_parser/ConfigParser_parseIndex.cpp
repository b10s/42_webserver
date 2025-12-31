#include "ConfigParser.hpp"

bool ConfigParser::IsSafeIndexFilename(const std::string& filename) const {
  if (filename.empty()) return false;
  if (filename.find("..") != std::string::npos) return false;
  if (filename.find('/') != std::string::npos)
    return false;  // absolute path or subdir
  for (size_t i = 0; i < filename.size(); ++i) {
    unsigned char c = static_cast<unsigned char>(filename[i]);
    if (!std::isprint(c)) return false;  // non-visible ASCII
    // reject unsafe chars such as spaces, quotes, angle brackets, etc.
    if (!(std::isalnum(c) || c == '0' || c == '.' || c == '_')) return false;
  }
  return true;
}

void ConfigParser::ParseIndex(Location* location) {
  std::string token = Tokenize(content);
  if (token.empty()) {
    throw std::runtime_error("Syntax error : expected index file name" + token);
  }
  if (IsSafeIndexFilename(token) == false) {
    throw std::runtime_error("Unsafe index file name: " + token);
  }
  location->SetIndexFile(token);
  ConsumeExpectedSemicolon("index file name");
}
