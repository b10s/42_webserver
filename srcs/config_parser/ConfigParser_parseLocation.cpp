#include "ConfigParser.hpp"

/*
I made the trailing slash in Location names optional for simplicity.
Previously, we enforced a trailing slash,
but that introduces cases where URI normalization or redirects
become necessary (e.g. handling /kapouet and /kapouet/).
*/
void ConfigParser::ParseLocation(ServerConfig* server) {
  (void)server;
  std::string token;
  Location location = Location();

  token = Tokenize(content);
  if (token.empty() || token[0] != '/') {
    throw std::runtime_error("Invalid location name: " + token);
  }
  location.SetName(token); // TODO: reject '..', "//" or other dangerous names
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
