#include "config_parser.hpp"

// Extracts the next token (word or symbol) from the configuration file content.
void ConfigParser::parseLocation(ServerConfig* server) {
  (void)server;
  std::string token;
  Location location = Location();

  token = tokenize(content_);
  if (token.empty() || token[0] != '/' || token[token.size() - 1] != '/')
    throw std::runtime_error("Invalid location name: " + token);
  location.setName(token);
  token = tokenize(content_);
  if (token != "{")
    throw std::runtime_error(
        "Syntax error: expected '{' after location name, got: " + token);
  while (true) {
    token = tokenize(content_);
    if (token == ConfigTokens::ALLOW_METHODS)
      this->parseMethods(&location);
    else if (token == ConfigTokens::ROOT)
      this->parseRoot(&location);
    else if (token == ConfigTokens::AUTOINDEX)
      this->parseAutoIndex(&location);
    else if (token == ConfigTokens::INDEX)
      this->parseIndex(&location);
    else if (token == ConfigTokens::EXTENSION)
      this->parseExtensions(&location);
    else if (token == ConfigTokens::UPLOAD_PATH)
      this->parseUploadPath(&location);
    else if (token == ConfigTokens::REDIRECT)
      this->parseRedirect(&location);
    else if (token == ConfigTokens::CGI_PATH)
      this->parseCgiPath(&location);
    else
      break;
  }
  if (token != "}")
    throw std::runtime_error(
        "Syntax error: expected '}' at the end of location block, got: " +
        token);
  server->addLocation(location);
}