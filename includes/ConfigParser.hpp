#ifndef CONFIG_PARSER_HPP_
#define CONFIG_PARSER_HPP_

#include <sys/stat.h>

#include <cstdlib>
#include <cstring>
#include <fstream>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "Location.hpp"
#include "ServerConfig.hpp"
#include "enums.hpp"

namespace ConfigTokens {
const std::string SERVER = "server";
const std::string LOCATION = "location";
const std::string ERROR_PAGE = "error_page";
const std::string MAX_BODY = "client_max_body_size";
const std::string LISTEN = "listen";
const std::string SERVER_NAME = "server_name";
const std::string ROOT = "root";
const std::string INDEX = "index";
const std::string AUTOINDEX = "autoindex";
const std::string ALLOW_METHODS = "allow_methods";
const std::string CGI_PATH = "cgi_path";
const std::string REDIRECT = "return";
const std::string EXTENSION = "extension";
const std::string UPLOAD_PATH = "upload_path";
}  // namespace ConfigTokens

namespace UrlConstants {
const std::string kHttpsPrefix = "https://";
const std::string kHttpPrefix = "http://";
}  // namespace UrlConstants

class ConfigParser {
 private:
  size_t currentPos_;
  std::vector<ServerConfig> serverConfigs_;
  bool isValidPortNumber(const std::string& port) const;
  bool isAllDigits(const std::string& str) const;
  bool isDirective(const std::string& token) const;
  TokenType toTokenType(const std::string& token) const;
  std::string tokenize(const std::string& content);

 public:
  std::string content_;  // Made public for easier access in parsing functions
  ConfigParser();        // Default constructor for tests
  explicit ConfigParser(const std::string& text);
  ~ConfigParser();
  void loadFile(const std::string& filename);

  void parse();
  void parseServer();
  void parseListen(ServerConfig* serverConfig);
  void parseServerName(ServerConfig* serverConfig);
  void parseMaxBody(ServerConfig* serverConfig);
  void parseErrorPage(ServerConfig* serverConfig);

  void parseLocation(ServerConfig* serverConfig);
  void parseMethods(Location* location);
  void parseRoot(Location* location);
  void parseAutoIndex(Location* location);
  void parseIndex(Location* location);
  void parseExtensions(Location* location);
  void parseUploadPath(Location* location);
  void parseRedirect(Location* location);
  void parseCgiPath(Location* location);
  template <typename T, typename Setter>
  void parseSimpleDirective(T* obj, Setter setter, const std::string& errorMsg);

  const std::vector<ServerConfig>& getServerConfigs() const {
    return serverConfigs_;
  }
};

template <typename T, typename Setter>
void ConfigParser::parseSimpleDirective(T* obj, Setter setter,
                                        const std::string& errorMsg) {
  std::string token = tokenize(content_);
  if (token.empty()) {
    throw std::runtime_error("Syntax error: expected " + errorMsg);
  }
  (obj->*setter)(token);
  token = tokenize(content_);
  if (token != ";") {
    throw std::runtime_error("Syntax error: expected ';' after " + errorMsg);
  }
}

#endif  // CONFIG_PARSER_HPP_

// array of array of string and int

// [
//   {127.0.0.1, 8080},
//   {192.168.0.1, 80},
//   ...
// ]

// test_conf new ServerConfig();

// test_conf.host_ == "asdsad"
// test_conf.port_ == "8080"
