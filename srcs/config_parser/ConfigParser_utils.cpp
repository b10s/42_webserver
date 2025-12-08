#include <cctype>

#include "ConfigParser.hpp"

namespace {
const std::string kWhitespace = " \t\r\n";
const std::string kSpecialLetter = "{};";
}  // namespace

namespace {
std::map<std::string, TokenType> CreateTokenTable() {
  std::map<std::string, TokenType> m;
  m.insert(std::make_pair(config_tokens::kListen, kTokenListen));
  m.insert(std::make_pair(config_tokens::kServerName, kTokenServerName));
  m.insert(std::make_pair(config_tokens::kMaxBody, kTokenMaxBody));
  m.insert(std::make_pair(config_tokens::kErrorPage, kTokenErrorPage));
  m.insert(std::make_pair(config_tokens::kLocation, kTokenLocation));
  m.insert(std::make_pair(config_tokens::kAllowMethods, kTokenAllowMethods));
  m.insert(std::make_pair(config_tokens::kRoot, kTokenRoot));
  m.insert(std::make_pair(config_tokens::kAutoIndex, kTokenAutoindex));
  m.insert(std::make_pair(config_tokens::kIndex, kTokenIndex));
  m.insert(std::make_pair(config_tokens::kExtension, kTokenExtension));
  m.insert(std::make_pair(config_tokens::kUploadPath, kTokenUploadPath));
  m.insert(std::make_pair(config_tokens::kRedirect, kTokenRedirect));
  m.insert(std::make_pair(config_tokens::kCgiPath, kTokenCgiPath));
  return m;
}

const std::map<std::string, TokenType> kTokenTable = CreateTokenTable();
}  // namespace

TokenType ConfigParser::ToTokenType(const std::string& token) const {
  std::map<std::string, TokenType>::const_iterator it = kTokenTable.find(token);
  if (it != kTokenTable.end()) return it->second;
  return kTokenUnknown;
}

// Extracts the next token (word or symbol) from the configuration file content.
// Parameters: content: The entire configuration file as a string
// Returns: The next token string
std::string ConfigParser::Tokenize(const std::string& content) {
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

bool ConfigParser::IsValidPortNumber(const std::string& port) const {
  if (port.empty() || port.length() > 5) return false;
  for (size_t i = 0; i < port.length(); ++i) {
    if (!isdigit(static_cast<unsigned char>(port[i]))) return false;
  }
  int port_num = std::atoi(port.c_str());
  return port_num > 0 && port_num <= 65535;
}

bool ConfigParser::IsAllDigits(const std::string& str) const {
  for (size_t i = 0; i < str.length(); ++i) {
    if (!isdigit(str[i])) return false;
  }
  return true;
}

bool ConfigParser::IsDirective(const std::string& token) const {
  return ToTokenType(token) != kTokenUnknown;
}

void ConfigParser::ConsumeExpectedSemicolon(const std::string& directive_name) {
  std::string token = Tokenize(content);
  if (token != ";") {
    throw std::runtime_error("Expected ';' after " + directive_name +
                             " directive");
  }
}
