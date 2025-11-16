#include "ConfigParser.hpp"

void ConfigParser::Parse() {
  std::string token;
  while (true) {
    token = Tokenize(content_);
    if (token.empty()) break;
    if (token == config_tokens::kServer) {
      ParseServer();
    } else {
      throw std::runtime_error("Syntax error: " + token);
    }
  }
}

void ConfigParser::ParseServer() {
  std::string token = Tokenize(content_);
  if (token != "{") {
    throw std::runtime_error("Syntax error: " + token);
  }
  ServerConfig server_config = ServerConfig();
  while (true) {
    token = Tokenize(content_);
    if (token == "}") break;
    switch (ToTokenType(token)) {
      case TOKEN_LISTEN:
        ParseListen(&server_config);
        break;
      case TOKEN_SERVER_NAME:
        ParseServerName(&server_config);
        break;
      case TOKEN_MAX_BODY:
        ParseMaxBody(&server_config);
        break;
      case TOKEN_ERROR_PAGE:
        ParseErrorPage(&server_config);
        break;
      case TOKEN_LOCATION:
        ParseLocation(&server_config);
        break;
      default:
        throw std::runtime_error("Unknown directive: " + token);
    }
  }
  server_configs_.push_back(server_config);
}
