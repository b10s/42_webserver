#ifndef CONFIG_HPP_
#define CONFIG_HPP_

#include <cstring>
#include <map>
#include <string>
#include <vector>
#include <string>
#include <stdexcept>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include "location.hpp"
#include "enums.hpp"
#include "location.hpp"

#define WIHTESPACE " \t\n"
#define SPECIAL_LETTERS "{};"

namespace {
  const std::string SERVER = "server";
  const std::string LOCATION = "location";
  const std::string ERROR_PAGE = "errorpage";
  const std::string MAX_BODY = "maxbody";
  const std::string HOST = "host";
  const std::string PORT = "port";
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

class ServerConfig {
 private:
  std::string host_;
  std::string port_;
  std::string serverName_;
  int maxBodySize_;
  std::map<HttpStatus, std::string> errors_;
  std::vector<Location> locations_;

 public:
  ServerConfig();
  void setHost(const std::string &host);
  void setPort(const std::string &port);
  void setServerName(const std::string &serverName);
  void setMaxBodySize(int size);
  const std::string getHost() const { return host_; }
  const std::string getPort() const { return port_; }
  const std::string getServerName() const { return serverName_; }
  int getMaxBodySize() const { return maxBodySize_; }
};

// maybe we can merge Config and ServerConfig later
// but for now, keep them separate for clarity
class Config {
 private:
  size_t currentPos_;
  std::string content_;
  std::vector<ServerConfig> serverConfigs_;

 public:
  Config(const std::string &filename);
  std::string tokenize(const std::string &content);
  void parse();
  void parseServer();
  void parseHost(ServerConfig *serverConfig);
  void parsePort(ServerConfig *serverConfig);
  void parseServerName(ServerConfig *serverConfig);
  void parseMaxBody(ServerConfig *serverConfig);
  const std::vector<ServerConfig>& getServerConfigs() const { return serverConfigs_; }
};

#endif  // CONFIG_HPP_

// array of array of string and int

// [
//   {127.0.0.1, 8080},
//   {192.168.0.1, 80},
//   ...
// ]

// test_conf new ServerConfig();

// test_conf.host_ == "asdsad"
// test_conf.port_ == "8080"
