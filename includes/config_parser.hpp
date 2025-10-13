#ifndef CONFIG_PARSER_HPP_
#define CONFIG_PARSER_HPP_

#include <cstring>
#include <map>
#include <string>
#include <vector>
#include <stdexcept>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include "server_config.hpp"
#include "location.hpp"
#include "enums.hpp"
#include <sys/stat.h>

#define WIHTESPACE " \t\n"
#define SPECIAL_LETTERS "{};"

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
}

namespace UrlConstants {
  const std::string kHttpsPrefix = "https://";
  const std::string kHttpPrefix = "http://";
}

// maybe we can merge Config and ServerConfig later
// but for now, keep them separate for clarity
class ConfigParser {
 private:
  size_t currentPos_;
  std::string content_;
  std::vector<ServerConfig> serverConfigs_;

 public:
  ConfigParser(const std::string &filename);
  std::string tokenize(const std::string &content);
  void parse();
  void parseServer();
  void parseListen(ServerConfig *serverConfig);
  void parseServerName(ServerConfig *serverConfig);
  void parseMaxBody(ServerConfig *serverConfig);
  void parseErrorPage(ServerConfig *serverConfig);
  // void parseLocation(ServerConfig *serverConfig);
  const std::vector<ServerConfig>& getServerConfigs() const { return serverConfigs_; }
  bool isValidPortNumber(const std::string &port) const;
  bool isAllDigits(const std::string &str) const;
};

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
