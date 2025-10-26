#include "ConfigParser.hpp"
#include <cctype>

namespace {
const std::string kWhitespace      = " \t\r\n";
const std::string kSpecialLetter  = "{};";
}

namespace {
std::map<std::string, TokenType> createTokenTable() {
  std::map<std::string, TokenType> m;
  m.insert(std::make_pair(ConfigTokens::LISTEN, TOKEN_LISTEN));
  m.insert(std::make_pair(ConfigTokens::SERVER_NAME, TOKEN_SERVER_NAME));
  m.insert(std::make_pair(ConfigTokens::MAX_BODY, TOKEN_MAX_BODY));
  m.insert(std::make_pair(ConfigTokens::ERROR_PAGE, TOKEN_ERROR_PAGE));
  m.insert(std::make_pair(ConfigTokens::LOCATION, TOKEN_LOCATION));
  m.insert(std::make_pair(ConfigTokens::ALLOW_METHODS, TOKEN_ALLOW_METHODS));
  m.insert(std::make_pair(ConfigTokens::ROOT, TOKEN_ROOT));
  m.insert(std::make_pair(ConfigTokens::AUTOINDEX, TOKEN_AUTOINDEX));
  m.insert(std::make_pair(ConfigTokens::INDEX, TOKEN_INDEX));
  m.insert(std::make_pair(ConfigTokens::EXTENSION, TOKEN_EXTENSION));
  m.insert(std::make_pair(ConfigTokens::UPLOAD_PATH, TOKEN_UPLOAD_PATH));
  m.insert(std::make_pair(ConfigTokens::REDIRECT, TOKEN_REDIRECT));
  m.insert(std::make_pair(ConfigTokens::CGI_PATH, TOKEN_CGI_PATH));
  return m;
}
const std::map<std::string, TokenType> kTokenTable = createTokenTable();
}

TokenType ConfigParser::toTokenType(const std::string& token) const {
  std::map<std::string, TokenType>::const_iterator it = kTokenTable.find(token);
  if (it != kTokenTable.end())
    return it->second;
  return TOKEN_UNKNOWN;
}

// Extracts the next token (word or symbol) from the configuration file content.
// Parameters: content: The entire configuration file as a string
// Returns: The next token string
std::string ConfigParser::tokenize(const std::string& content) {
  if (currentPos_ >= content.size()) return "";

  while (currentPos_ < content.size() &&
         kWhitespace.find(content[currentPos_]) != std::string::npos)
    currentPos_++;
  if (currentPos_ >= content.size()) return "";
  // handle special characters as single character tokens
  if (kSpecialLetter.find(content[currentPos_]) != std::string::npos)
    return std::string(1, content[currentPos_++]);
  // extract regular token
  size_t start = currentPos_;
  while (currentPos_ < content.size() &&
         kWhitespace.find(content[currentPos_]) == std::string::npos &&
         kSpecialLetter.find(content[currentPos_]) == std::string::npos)
    currentPos_++;
  return content.substr(start, currentPos_ - start);
}

bool ConfigParser::isValidPortNumber(const std::string& port) const {
  if (port.empty() || port.length() > 5) return false;
  for (size_t i = 0; i < port.length(); ++i) {
    if (!isdigit(static_cast<unsigned char>(port[i]))) return false;
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
  return toTokenType(token) != TOKEN_UNKNOWN;
}
