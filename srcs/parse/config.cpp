#include "config.hpp"

// Parses the entire configuration file and builds the server configuration objects.
Config::Config(const std::string &filename) : currentPos_(0) {
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
    if (token == ConfigTokens::SERVER) {
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
    if (token == ConfigTokens::HOST)
      this->parseHost(&serverConfig);
    else if (token == ConfigTokens::LISTEN)
      this->parsePort(&serverConfig);
    else if (token == ConfigTokens::SERVER_NAME)
      this->parseServerName(&serverConfig);
    else if (token == ConfigTokens::MAX_BODY)
      this->parseMaxBody(&serverConfig);
    else if (token == ConfigTokens::ERROR_PAGE)
      this->parseErrorPage(&serverConfig);
    else if (token == ConfigTokens::ROOT)
      ; // this->parseLocation(&serverConfig);
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

/*
Internal URI (e.g., /404.html, /error/default.html)
  Processed by nginx as an internal redirect within the same server.
External URL (scheme + host + path, e.g., http://example.com/404.html)
  Triggers an external redirect (e.g., 302) sent to the client.
  The browser is redirected to another domain or server.
Relative Path (e.g., ./404.html, ../404.html)
  Invalid in nginx configuration. Cannot be matched in nginx's routing mechanism.
*/
void Config::parseErrorPage(ServerConfig *serverConfig) {
  std::string token = tokenize(content_);
  if (token.empty()) {
    throw std::runtime_error("Syntax error : expected error code" + token);
  }
  std::string errorCodeStr = token;
  int errorCode = std::atoi(token.c_str());
  if (errorCode < 400 || errorCode > 599) {
    throw std::runtime_error("Invalid error code: " + token);
  }

  token = tokenize(content_);
  if (token.empty()) {
    throw std::runtime_error("Syntax error : expected error page path" + token);
  }
  if (token[0] == '.') {
    throw std::runtime_error("Error page path must be absolute: " + token);
  }
  if (token.find(UrlConstants::kHttpsPrefix) != 0 &&
      token.find(UrlConstants::kHttpPrefix) != 0 &&
      token[0] != '/') {
    token = "/" + token;
  }
  serverConfig->setErrorPage(static_cast<HttpStatus>(errorCode), token);
  token = tokenize(content_);
  if (token != ";") {
    throw std::runtime_error("Syntax error: expected ';' after error_page directive" + token);
  }
}
