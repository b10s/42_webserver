#include "ConfigParser.hpp"

// Extracts the next token (word or symbol) from the configuration file content.
void ConfigParser::ParseLocation(ServerConfig* server) {
  (void)server;
  std::string token;
  Location location = Location();

  token = Tokenize(content_);
  if (token.empty() || token[0] != '/' || token[token.size() - 1] != '/')
    throw std::runtime_error("Invalid location name: " + token);
  location.SetName(token);
  token = Tokenize(content_);
  if (token != "{")
    throw std::runtime_error(
        "Syntax error: expected '{' after location name, got: " + token);
  while (true) {
    token = Tokenize(content_);
    if (token == "}") break;
    switch (ToTokenType(token)) {
      case TOKEN_ALLOW_METHODS:
        ParseMethods(&location);
        break;
      case TOKEN_ROOT:
        ParseRoot(&location);
        break;
      case TOKEN_AUTOINDEX:
        ParseAutoIndex(&location);
        break;
      case TOKEN_INDEX:
        ParseIndex(&location);
        break;
      case TOKEN_EXTENSION:
        ParseExtensions(&location);
        break;
      case TOKEN_UPLOAD_PATH:
        ParseUploadPath(&location);
        break;
      case TOKEN_REDIRECT:
        ParseRedirect(&location);
        break;
      case TOKEN_CGI_PATH:
        ParseCgiPath(&location);
        break;
      default:
        throw std::runtime_error("Unknown directive in location: " + token);
    }
  }
  server->AddLocation(location);
}
