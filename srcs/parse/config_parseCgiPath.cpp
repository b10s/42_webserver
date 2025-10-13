#include "config_parser.hpp"

// Extracts the next token (word or symbol) from the configuration file content.
void ConfigParser::parseCgiPath(Location *location) {
  std::string token;

  token = tokenize(content_);
  location->setCgiPath(token);
  token = tokenize(content_);
  if (token != ";") throw std::runtime_error("Syntax error: expected ';' after cgi_path value" + token);
}
