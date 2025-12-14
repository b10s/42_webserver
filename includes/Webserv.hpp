#ifndef WEBSERV_HPP
#define WEBSERV_HPP

#include <iostream>

#include "ConfigParser.hpp"
#include "HttpRequest.hpp"
#include "HttpResponse.hpp"
#include "ServerConfig.hpp"

class Webserv {
 private:
  std::map<std::string, ServerConfig> port_to_server_configs_;

  // for event loop
  std::map<int, time_t> client_last_activity_;  // for timeouts
  std::map<int, std::pair<HttpResponse, bool> >
      outbound_responses_;  // response and whether headers sent
  std::map<int, pid_t> cgi_fd_to_pid_;
  std::map<int, int> cgi_fd_to_client_fd_;
  std::map<int, bool> client_fd_to_keep_alive_;
  std::set<int> keep_alive_fds_;

  void InitServersFromConfigs(const std::vector<ServerConfig>& server_configs);

 public:
  Webserv();
  Webserv(const std::string& config_file);
  ~Webserv();

  // utility methods for accessing server configurations
  const std::map<std::string, ServerConfig>& GetPortConfigs() const;
  const ServerConfig* FindServerConfigByPort(const std::string& port) const;
  void HandleRequest(int client_fd, const std::string& port);

  // for testing purposes
  void TestConfiguration();
};

#endif  // WEBSERV_HPP
