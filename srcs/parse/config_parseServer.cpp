#include "config_parser.hpp"

// Parses the entire configuration file and builds the server configuration objects.
ConfigParser::ConfigParser(const std::string &filename) : currentPos_(0) {
    std::ifstream file(filename.c_str());
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file: " + filename);
    }

    struct stat s;
    if (stat(filename.c_str(), &s) == 0) {
      if (s.st_mode & S_IFDIR) {
        throw std::runtime_error(filename + " is a directory");
      }
    } else {
      throw std::runtime_error("Failed to get file status: " + filename);
    }

    this->content_ = std::string(
      std::istreambuf_iterator<char>(file),
      std::istreambuf_iterator<char>()
    );
    this->parse();
}

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
      ; // this->parseLocation(&serverConfig);
    else
      break;
  }
  if (token != "}") {
    throw std::runtime_error("Syntax error: " + token);
  }
  this->serverConfigs_.push_back(serverConfig);
}
