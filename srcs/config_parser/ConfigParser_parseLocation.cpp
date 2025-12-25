#include "ConfigParser.hpp"

// Extracts the next token (word or symbol) from the configuration file content.
void ConfigParser::ParseLocation(ServerConfig* server) {
  (void)server;
  std::string token;
  Location location = Location();

  token = Tokenize(content);
  if (token.empty() || token[0] != '/' || token[token.size() - 1] != '/')
    throw std::runtime_error("Invalid location name: " + token);
  location.SetName(token);
  token = Tokenize(content);
  if (token != "{")
    throw std::runtime_error(
        "Syntax error: expected '{' after location name, got: " + token);
  while (true) {
    token = Tokenize(content);
    if (token == "}") break;
    switch (ToTokenType(token)) {
      case kTokenAllowMethods:
        ParseMethods(&location);
        break;
      case kTokenRoot:
        ParseRoot(&location);
        break;
      case kTokenAutoindex:
        ParseAutoIndex(&location);
        break;
      case kTokenIndex:
        ParseIndex(&location);
        break;
      case kTokenUploadPath:
        ParseUploadPath(&location);
        break;
      case kTokenRedirect:
        ParseRedirect(&location);
        break;
      case kTokenCgi:
        ParseCgi(&location);
        break;
      default:
        throw std::runtime_error("Unknown directive in location: " + token);
    }
  }
  server->AddLocation(location);
}
