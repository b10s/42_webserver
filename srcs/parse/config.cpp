#include "../../include/config.hpp"

// Parses the entire configuration file and builds the server configuration objects.
Config::Config(const std::string &filename) : currentPos_(0) {
    std::ifstream file(filename.c_str());
    if (!file) {
        throw std::runtime_error("Failed to open file: " + filename);
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string content = buffer.str();
    this->content_ = content;
    this->parse();
}

// Extracts the next token (word or symbol) from the configuration file content.
// Parameters: content: The entire configuration file as a string
// Returns: The next token string
std::string Config::tokenize(const std::string &content) {
  size_t start = currentPos_;

  while (content[currentPos_] && strchr(WIHTESPACE, content[currentPos_])) currentPos_++;
  if (content[currentPos_] == '\0') return "";
  // handle special characters as single character tokens
  if (strchr(SPECIAL_LETTERS, content[currentPos_]))
    return std::string(1, content[currentPos_++]);
  // extract regular token
  start = currentPos_;
  while (content[currentPos_] && !strchr(WIHTESPACE, content[currentPos_]) &&
         !strchr(SPECIAL_LETTERS, content[currentPos_]))
    currentPos_++;
  return content.substr(start, currentPos_ - start);
}

void Config::parse() {
  std::string token;
  while (true) {
    token = tokenize(content_);
    if (token.empty()) break;
    if (token == SERVER) {
      this->parseServer();
    } else {
      throw std::runtime_error("Syntax error: " + token);
    }
  }
}

void Config::parseServer() {
  std::string token = tokenize(content_);
  if (token != "{") {
    throw std::runtime_error("Syntax error: " + token);
  }

  ServerConfig serverConfig = ServerConfig();
  while (true) {
    token = tokenize(content_);
    if (token == HOST)
      this->parseHost(&serverConfig);
    else if (token == LISTEN)
      this->parsePort(&serverConfig);
    else if (token == SERVER_NAME)
      this->parseServerName(&serverConfig);
    else if (token == MAX_BODY)
      this->parseMaxBody(&serverConfig);
    else
      break;
  }
  if (token != "}") {
    throw std::runtime_error("Syntax error: " + token);
  }
  this->serverConfigs_.push_back(serverConfig);
}

void Config::parseHost(ServerConfig *serverConfig) {
  std::string token = tokenize(content_);
  if (token.empty()) {
    throw std::runtime_error("Syntax error : expected host value" + token);
  }
  serverConfig->setHost(token);
  token = tokenize(content_);
  if (token != ";") {
    throw std::runtime_error("Syntax error: expected ';' after host value" + token);
  }
}

void Config::parsePort(ServerConfig *serverConfig) {
  std::string token = tokenize(content_);
  if (token.empty()) {
    throw std::runtime_error("Syntax error : expected port value" + token);
  }
  serverConfig->setPort(token);
  token = tokenize(content_);
  if (token != ";") {
    throw std::runtime_error("Syntax error: expected ';' after port value" + token);
  }
}

void Config::parseServerName(ServerConfig *serverConfig) {
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

void Config::parseMaxBody(ServerConfig *serverConfig) {
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
    throw std::runtime_error("Syntax error: expected ';' after max_body value" + token);
  }
}

