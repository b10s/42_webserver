#ifndef WEBSERV_HPP
#define WEBSERV_HPP

#include <sys/epoll.h>

#include <iostream>

#include "ConfigParser.hpp"
#include "Epoll.hpp"
#include "HttpRequest.hpp"
#include "HttpResponse.hpp"
#include "ServerConfig.hpp"

class Webserv {
 private:
  std::map<unsigned short, ServerConfig> port_to_server_configs_;
  Epoll epoll_;

  // for event loop
  std::map<int, time_t> client_last_activity_;  // for timeouts
  std::map<int, std::string> output_buffers_;
  std::map<int, pid_t> cgi_fd_to_pid_;
  std::map<int, int> cgi_fd_to_client_fd_;
  std::map<int, bool> client_fd_to_keep_alive_;
  std::set<int> keep_alive_fds_;
  std::map<int, std::string> raw_requests_;

  static const size_t kBufferSize = 4096;

 public:
  Webserv();  // should be private but made public for testing
  Webserv(const std::string& config_file);
  ~Webserv();

  void Run();

  // InitServersFromConfigs should be private but made public for testing
  void InitServersFromConfigs(const std::vector<ServerConfig>& server_configs);

  // utility methods for accessing server configurations
  const std::map<unsigned short, ServerConfig>& GetPortConfigs() const;
  const ServerConfig* FindServerConfigByPort(const unsigned short& port) const;
  void HandleRequest(int client_fd, const unsigned short& port);

  // for testing purposes
  void TestConfiguration();
};

#endif  // WEBSERV_HPP
