#include "ConfigParser.hpp"

void ConfigParser::Parse() {
  std::string token;
  while (true) {
    token = Tokenize(content);
    if (token.empty()) break;
    if (token == config_tokens::kServer) {
      ParseServer();
    } else {
      throw std::runtime_error("Syntax error: " + token);
    }
  }
}

void ConfigParser::ParseServer() {
  std::string token = Tokenize(content);
  if (token != "{") {
    throw std::runtime_error("Syntax error: " + token);
  }
  ServerConfig server_config = ServerConfig();
  while (true) {
    token = Tokenize(content);
    if (token == "}") break;
    switch (ToTokenType(token)) {
      case kTokenListen:
        ParseListen(&server_config);
        break;
      case kTokenServerName:
        ParseServerName(&server_config);
        break;
      case kTokenMaxBody:
        ParseMaxBody(&server_config);
        break;
      case kTokenErrorPage:
        ParseErrorPage(&server_config);
        break;
      case kTokenLocation:
        ParseLocation(&server_config);
        break;
      default:
        throw std::runtime_error("Unknown directive: " + token);
    }
  }
  server_configs_.push_back(server_config);
}
