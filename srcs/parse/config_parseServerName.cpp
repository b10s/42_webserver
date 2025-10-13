#include "config_parser.hpp"

void ConfigParser::parseServerName(ServerConfig *serverConfig) {
  std::string token = tokenize(content_);
  if (token.empty()) {
    throw std::runtime_error("Syntax error : expected server_name value" + token);
  }
  serverConfig->setServerName(token);
  token = tokenize(content_);
  if (token != ";") {
    throw std::runtime_error("Syntax error: expected ';' after server_name value" + token);
  }
}
