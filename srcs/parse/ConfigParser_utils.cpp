#include <cctype>

#include "ConfigParser.hpp"

namespace {
const std::string kWhitespace = " \t\r\n";
const std::string kSpecialLetter = "{};";
}  // namespace

namespace {
std::map<std::string, TokenType> CreateTokenTable() {
  std::map<std::string, TokenType> m;
  m.insert(std::make_pair(config_tokens::kListen, TOKEN_LISTEN));
  m.insert(std::make_pair(config_tokens::kServerName, TOKEN_SERVER_NAME));
  m.insert(std::make_pair(config_tokens::kMaxBody, TOKEN_MAX_BODY));
  m.insert(std::make_pair(config_tokens::kErrorPage, TOKEN_ERROR_PAGE));
  m.insert(std::make_pair(config_tokens::kLocation, TOKEN_LOCATION));
  m.insert(std::make_pair(config_tokens::kAllowMethods, TOKEN_ALLOW_METHODS));
  m.insert(std::make_pair(config_tokens::kRoot, TOKEN_ROOT));
  m.insert(std::make_pair(config_tokens::kAutoIndex, TOKEN_AUTOINDEX));
  m.insert(std::make_pair(config_tokens::kIndex, TOKEN_INDEX));
  m.insert(std::make_pair(config_tokens::kExtension, TOKEN_EXTENSION));
  m.insert(std::make_pair(config_tokens::kUploadPath, TOKEN_UPLOAD_PATH));
  m.insert(std::make_pair(config_tokens::kRedirect, TOKEN_REDIRECT));
  m.insert(std::make_pair(config_tokens::kCgiPath, TOKEN_CGI_PATH));
  return m;
}

const std::map<std::string, TokenType> kTokenTable = CreateTokenTable();
}  // namespace

TokenType ConfigParser::toTokenType(const std::string& token) const {
  std::map<std::string, TokenType>::const_iterator it = kTokenTable.find(token);
  if (it != kTokenTable.end()) return it->second;
  return TOKEN_UNKNOWN;
}

// Extracts the next token (word or symbol) from the configuration file content.
// Parameters: content: The entire configuration file as a string
// Returns: The next token string
std::string ConfigParser::tokenize(const std::string& content) {
  if (current_pos_ >= content.size()) return "";

  while (current_pos_ < content.size() &&
         kWhitespace.find(content[current_pos_]) != std::string::npos)
    current_pos_++;
  if (current_pos_ >= content.size()) return "";
  // handle special characters as single character tokens
  if (kSpecialLetter.find(content[current_pos_]) != std::string::npos)
    return std::string(1, content[current_pos_++]);
  // extract regular token
  size_t start = current_pos_;
  while (current_pos_ < content.size() &&
         kWhitespace.find(content[current_pos_]) == std::string::npos &&
         kSpecialLetter.find(content[current_pos_]) == std::string::npos)
    current_pos_++;
  return content.substr(start, current_pos_ - start);
}

bool ConfigParser::isValidPortNumber(const std::string& port) const {
  if (port.empty() || port.length() > 5) return false;
  for (size_t i = 0; i < port.length(); ++i) {
    if (!isdigit(static_cast<unsigned char>(port[i]))) return false;
  }
  int port_num = std::atoi(port.c_str());
  return port_num > 0 && port_num <= 65535;
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
