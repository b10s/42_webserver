#ifndef CONFIG_HPP_
#define CONFIG_HPP_

#include <cstring>
#include <map>
#include <string>
#include <vector>

#include "enums.hpp"
#include "location.hpp"

#define WIHTESPACE " \t\n"
#define SPECIAL_LETTERS "{};"

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
};

// maybe we can merge Config and ServerConfig later
// but for now, keep them separate for clarity
class Config {
 private:
  std::string content_;
  std::vector<ServerConfig> serverConfigs_;

 public:
  Config(const std::string& filename);
  std::string tokenize(const std::string& content);
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
