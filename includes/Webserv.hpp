#ifndef WEBSERV_HPP
#define WEBSERV_HPP

#include <sys/epoll.h>

#include <iostream>
#include <map>
#include <vector>

#include "ConfigParser.hpp"
#include "ServerConfig.hpp"
#include "socket/ASocket.hpp"

class Webserv {
 private:
  std::map<unsigned short, ServerConfig> port_to_server_configs_;
  int epoll_fd_;
  std::map<int, ASocket*> sockets_;

  static const int kMaxEvents = 10;

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
};

#endif  // WEBSERV_HPP
