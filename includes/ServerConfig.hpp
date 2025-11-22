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

#include "Location.hpp"
#include "enums.hpp"

class ServerConfig {
 private:
  std::string host_;
  std::string port_;
  std::string server_name_;
  int max_body_size_;
  std::map<HttpStatus, std::string> errors_;
  std::vector<Location> locations_;
  bool has_listen_;
  bool has_server_name_;
  bool has_max_body_;

 public:
  ServerConfig();
  void SetHost(const std::string& host);
  void SetPort(const std::string& port);
  void SetServerName(const std::string& server_name);
  void SetMaxBodySize(int size);

  void SetErrorPage(HttpStatus status, const std::string& path) {
    errors_[status] = path;
  }

  const std::string GetHost() const {
    return host_;
  }

  const std::string GetPort() const {
    return port_;
  }

  const std::string GetServerName() const {
    return server_name_;
  }

  int GetMaxBodySize() const {
    return max_body_size_;
  }

  const std::map<HttpStatus, std::string>& GetErrorPages() const {
    return errors_;
  }

  std::string GetErrorPagesString() const {
    std::string result;
    for (std::map<HttpStatus, std::string>::const_iterator it = errors_.begin();
         it != errors_.end(); ++it) {
      std::ostringstream oss;
      oss << it->first;
      result += "    " + oss.str() + " -> " + it->second + "\n";
    }
    return result;
  }

  void AddLocation(const Location& location) {
    // check for duplicate location names
    for (size_t i = 0; i < locations_.size(); ++i) {
      if (locations_[i].GetName() == location.GetName()) {
        throw std::runtime_error("Duplicate location name: " +
                                 location.GetName());
      }
    }
    locations_.push_back(location);
  }

  const std::vector<Location>& GetLocations() const {
    return locations_;
  }
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
