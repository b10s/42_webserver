#include "ConfigParser.hpp"

void ConfigParser::parse() {
  std::string token;
  while (true) {
    token = Tokenize(content_);
    if (token.empty()) break;
    if (token == config_tokens::kServer) {
      parseServer();
    } else {
      throw std::runtime_error("Syntax error: " + token);
    }
  }
}

void ConfigParser::parseServer() {
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
        parseListen(&server_config);
        break;
      case TOKEN_SERVER_NAME:
        parseServerName(&server_config);
        break;
      case TOKEN_MAX_BODY:
        parseMaxBody(&server_config);
        break;
      case TOKEN_ERROR_PAGE:
        parseErrorPage(&server_config);
        break;
      case TOKEN_LOCATION:
        parseLocation(&server_config);
        break;
      default:
        throw std::runtime_error("Unknown directive: " + token);
    }
  }
  server_configs_.push_back(server_config);
}
