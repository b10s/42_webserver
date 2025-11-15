#include "ConfigParser.hpp"

void ConfigParser::parseMaxBody(ServerConfig* server_config) {
  std::string token = tokenize(content_);
  if (token.empty()) {
    throw std::runtime_error("Syntax error : expected max_body value" + token);
  }
  int size = std::atoi(token.c_str());
  if (size < 0 || size > 100000000) {
    throw std::runtime_error("Invalid max_body size: " + token);
  }
  server_config->setMaxBodySize(size);
  token = tokenize(content_);
  if (token != ";") {
    throw std::runtime_error("Syntax error: expected ';' after max_body value" +
                             token);
  }
}
