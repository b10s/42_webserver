#include "config_parser.hpp"

// Extracts the next token (word or symbol) from the configuration file content.
// Parameters: content: The entire configuration file as a string
// Returns: The next token string
std::string ConfigParser::tokenize(const std::string& content) {
  if (currentPos_ >= content.size()) return "";

  while (currentPos_ < content.size() &&
         strchr(WHITESPACE, content[currentPos_]))
    currentPos_++;
  if (currentPos_ >= content.size()) return "";
  // handle special characters as single character tokens
  if (strchr(SPECIAL_LETTERS, content[currentPos_]))
    return std::string(1, content[currentPos_++]);
  // extract regular token
  size_t start = currentPos_;
  while (content[currentPos_] && !strchr(WHITESPACE, content[currentPos_]) &&
         !strchr(SPECIAL_LETTERS, content[currentPos_]))
    currentPos_++;
  return content.substr(start, currentPos_ - start);
}

bool ConfigParser::isValidPortNumber(const std::string& port) const {
  if (port.empty() || port.length() > 5) return false;
  for (size_t i = 0; i < port.length(); ++i) {
    if (!isdigit(port[i])) return false;
  }
  int portNum = std::atoi(port.c_str());
  return portNum > 0 && portNum <= 65535;
}

bool ConfigParser::isAllDigits(const std::string& str) const {
  for (size_t i = 0; i < str.length(); ++i) {
    if (!isdigit(str[i])) return false;
  }
  return true;
}

bool ConfigParser::isDirective(const std::string& token) const {
  return (token == ConfigTokens::SERVER || token == ConfigTokens::LOCATION ||
          token == ConfigTokens::ERROR_PAGE ||
          token == ConfigTokens::MAX_BODY || token == ConfigTokens::LISTEN ||
          token == ConfigTokens::SERVER_NAME || token == ConfigTokens::ROOT ||
          token == ConfigTokens::INDEX || token == ConfigTokens::AUTOINDEX ||
          token == ConfigTokens::ALLOW_METHODS ||
          token == ConfigTokens::CGI_PATH || token == ConfigTokens::REDIRECT ||
          token == ConfigTokens::EXTENSION ||
          token == ConfigTokens::UPLOAD_PATH);
}
