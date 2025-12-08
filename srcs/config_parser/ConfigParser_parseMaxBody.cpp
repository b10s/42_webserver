#include "ConfigParser.hpp"

void ConfigParser::ParseMaxBody(ServerConfig* server_config) {
  std::string token = Tokenize(content);
  if (token.empty()) {
    throw std::runtime_error("Syntax error : expected max_body value" + token);
  }
  int size = std::atoi(token.c_str());
  if (size < 0 || size > 100000000) {
    throw std::runtime_error("Invalid max_body size: " + token);
  }
  server_config->SetMaxBodySize(size);
  ConsumeExpectedSemicolon("max_body");
}
