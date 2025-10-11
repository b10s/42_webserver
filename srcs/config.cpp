#include "config.hpp"

// Parses the entire configuration file and builds the server configuration
// objects.
Config::Config(const std::string& filename) { (void)filename; }

// void Config::parse() {
// }

// Extracts the next token (word or symbol) from the configuration file content.
// Parameters: content: The entire configuration file as a string
// Returns: The next token string
std::string Config::tokenize(const std::string& content) {
  static size_t pos = 0;
  size_t start = pos;

  while (content[pos] && strchr(WIHTESPACE, content[pos])) pos++;
  if (content[pos] == '\0') return "";
  // handle special characters as single character tokens
  if (strchr(SPECIAL_LETTERS, content[pos]))
    return std::string(1, content[pos++]);
  // extract regular token
  start = pos;
  while (content[pos] && !strchr(WIHTESPACE, content[pos]) &&
         !strchr(SPECIAL_LETTERS, content[pos]))
    pos++;
  return content.substr(start, pos - start);
}
