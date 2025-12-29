#ifndef SERVER_CONFIG_HPP_
#define SERVER_CONFIG_HPP_

#include <sys/stat.h>

#include <cstdlib>
#include <cstring>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "Location.hpp"
#include "lib/http/Status.hpp"
#include "LocationMatch.hpp"

class ServerConfig {
 private:
  std::string host_;
  unsigned short port_;
  std::string server_name_;
  int max_body_size_;
  std::map<lib::http::Status, std::string> errors_;
  std::vector<Location> locations_;
  bool has_listen_;
  bool has_server_name_;
  bool has_max_body_;
  static std::string TrimTrailingSlashExceptRoot(const std::string& s);
  bool IsPathPrefix(const std::string& uri, const std::string& prefix) const;

 public:
  ServerConfig();
  void SetListen(const std::string& host, const unsigned short& port);
  void SetHost(const std::string& host);
  void SetPort(const unsigned short& port);
  void SetServerName(const std::string& server_name);
  void SetMaxBodySize(int size);
  LocationMatch FindLocationForUri(const std::string& uri) const;

  void SetErrorPage(lib::http::Status status, const std::string& path) {
    errors_[status] = path;
  }

  const std::string& GetHost() const {
    return host_;
  }

  const unsigned short& GetPort() const {
    return port_;
  }

  const std::string& GetServerName() const {
    return server_name_;
  }

  int GetMaxBodySize() const {
    return max_body_size_;
  }

  const std::map<lib::http::Status, std::string>& GetErrorPages() const {
    return errors_;
  }

  std::string GetErrorPagesString() const {
    std::string result;
    for (std::map<lib::http::Status, std::string>::const_iterator it =
             errors_.begin();
         it != errors_.end(); ++it) {
      std::ostringstream oss;
      oss << it->first;
      result += "    " + oss.str() + " -> " + it->second + "\n";
    }
    return result;
  }

  /*
  check for duplicate location names after normalization
  /img and /img/ are considered the same
  */
  void AddLocation(const Location& location) {
    const std::string normalized_name =
        TrimTrailingSlashExceptRoot(location.GetName());
    for (size_t i = 0; i < locations_.size(); ++i) {
      const std::string existing_name =
          TrimTrailingSlashExceptRoot(locations_[i].GetName());
      if (existing_name == normalized_name) {
        throw std::runtime_error("Duplicate location name: " + normalized_name);
      }
    }
    locations_.push_back(location);
  }

  const std::vector<Location>& GetLocations() const {
    return locations_;
  }
};

#endif
