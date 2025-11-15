#include "ConfigParser.hpp"

// Extracts the next token (word or symbol) from the configuration file content.
void ConfigParser::parseLocation(ServerConfig* server) {
  (void)server;
  std::string token;
  Location location = Location();

  token = tokenize(content_);
  if (token.empty() || token[0] != '/' || token[token.size() - 1] != '/')
    throw std::runtime_error("Invalid location name: " + token);
  location.SetName(token);
  token = tokenize(content_);
  if (token != "{")
    throw std::runtime_error(
        "Syntax error: expected '{' after location name, got: " + token);
  while (true) {
    token = tokenize(content_);
    if (token == "}") break;
    switch (toTokenType(token)) {
      case TOKEN_ALLOW_METHODS:
        consumeMethods(&location);
        break;
      case TOKEN_ROOT:
        parseRoot(&location);
        break;
      case TOKEN_AUTOINDEX:
        parseAutoIndex(&location);
        break;
      case TOKEN_INDEX:
        parseIndex(&location);
        break;
      case TOKEN_EXTENSION:
        parseExtensions(&location);
        break;
      case TOKEN_UPLOAD_PATH:
        parseUploadPath(&location);
        break;
      case TOKEN_REDIRECT:
        parseRedirect(&location);
        break;
      case TOKEN_CGI_PATH:
        parseCgiPath(&location);
        break;
      default:
        throw std::runtime_error("Unknown directive in location: " + token);
    }
  }
  server->addLocation(location);
}
