#ifndef WEBSERV_HPP
#define WEBSERV_HPP

#include <sys/epoll.h>

#include <map>
#include <vector>

#include "ServerConfig.hpp"
#include "socket/ASocket.hpp"

class Webserv {
 private:
  std::map<unsigned short, ServerConfig> port_to_server_configs_;
  lib::type::Fd epoll_fd_;
  std::map<int, ASocket*> sockets_;
  void ClearResources();

  static const int kMaxEvents = 10;
  static const int kRequestTimeout = 10;
  void CheckTimeout();

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
