#include "config_parser.hpp"

// Extracts the next token (word or symbol) from the configuration file content.
void ConfigParser::parseUploadPath(Location *location) {
  std::string token;

  token = tokenize(content_);
  location->setUploadPath(token);
  token = tokenize(content_);
  if (token != ";") throw std::runtime_error("Syntax error: expected ';' after upload_path value" + token);
}
