#include "ConfigParser.hpp"

void ConfigParser::parse() {
  std::string token;
  while (true) {
    token = tokenize(content_);
    if (token.empty()) break;
    if (token == config_tokens::kServer) {
      this->parseServer();
    } else {
      throw std::runtime_error("Syntax error: " + token);
    }
  }
}

void ConfigParser::parseServer() {
  std::string token = tokenize(content_);
  if (token != "{") {
    throw std::runtime_error("Syntax error: " + token);
  }
  ServerConfig serverConfig = ServerConfig();
  while (true) {
    token = tokenize(content_);
    if (token == "}") break;
    switch (toTokenType(token)) {
      case TOKEN_LISTEN:
        parseListen(&serverConfig);
        break;
      case TOKEN_SERVER_NAME:
        parseServerName(&serverConfig);
        break;
      case TOKEN_MAX_BODY:
        parseMaxBody(&serverConfig);
        break;
      case TOKEN_ERROR_PAGE:
        parseErrorPage(&serverConfig);
        break;
      case TOKEN_LOCATION:
        parseLocation(&serverConfig);
        break;
      default:
        throw std::runtime_error("Unknown directive: " + token);
    }
  }
  this->server_configs_.push_back(serverConfig);
}
