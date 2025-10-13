#include "config_parser.hpp"

void ConfigParser::parse() {
  std::string token;
  while (true) {
    token = tokenize(content_);
    if (token.empty()) break;
    if (token == ConfigTokens::SERVER) {
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
    if (token == ConfigTokens::LISTEN)
      this->parseListen(&serverConfig);
    else if (token == ConfigTokens::SERVER_NAME)
      this->parseServerName(&serverConfig);
    else if (token == ConfigTokens::MAX_BODY)
      this->parseMaxBody(&serverConfig);
    else if (token == ConfigTokens::ERROR_PAGE)
      this->parseErrorPage(&serverConfig);
    else if (token == ConfigTokens::LOCATION)
      parseLocation(&serverConfig);
    else
      break;
  }
  if (token != "}") {
    throw std::runtime_error("Syntax error: " + token);
  }
  this->serverConfigs_.push_back(serverConfig);
}
