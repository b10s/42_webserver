#ifndef SERVER_CONFIG_HPP_
#define SERVER_CONFIG_HPP_

#include <sys/stat.h>

#include <cstdlib>
#include <cstring>
#include <fstream>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "enums.hpp"
#include "location.hpp"

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
  void setHost(const std::string& host);
  void setPort(const std::string& port);
  void setServerName(const std::string& serverName);
  void setMaxBodySize(int size);
  void setErrorPage(HttpStatus status, const std::string& path) {
    errors_[status] = path;
  }
  const std::string getHost() const { return host_; }
  const std::string getPort() const { return port_; }
  const std::string getServerName() const { return serverName_; }
  int getMaxBodySize() const { return maxBodySize_; }
  const std::map<HttpStatus, std::string>& getErrorPages() const {
    return errors_;
  }
  std::string getErrorPagesString() const {
    std::string result;
    for (std::map<HttpStatus, std::string>::const_iterator it = errors_.begin();
         it != errors_.end(); ++it) {
      std::ostringstream oss;
      oss << it->first;
      result += "    " + oss.str() + " -> " + it->second + "\n";
    }
    return result;
  }
  void addLocation(const Location& location) { locations_.push_back(location); }
  const std::vector<Location>& getLocations() const { return locations_; }
};

#endif

// array of array of string and int

// [
//   {127.0.0.1, 8080},
//   {192.168.0.1, 80},
//   ...
// ]

// test_conf new ServerConfig();

// test_conf.host_ == "asdsad"
// test_conf.port_ == "8080"
