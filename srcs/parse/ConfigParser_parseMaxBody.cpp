#include "ConfigParser.hpp"

void ConfigParser::parseMaxBody(ServerConfig* serverConfig) {
  std::string token = tokenize(content_);
  if (token.empty()) {
    throw std::runtime_error("Syntax error : expected max_body value" + token);
  }
  int size = std::atoi(token.c_str());
  if (size < 0 || size > 100000000) {
    throw std::runtime_error("Invalid max_body size: " + token);
  }
  serverConfig->setMaxBodySize(size);
  token = tokenize(content_);
  if (token != ";") {
    throw std::runtime_error("Syntax error: expected ';' after max_body value" +
                             token);
  }
}
