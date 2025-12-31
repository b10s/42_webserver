#include "Webserv.hpp"

#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>

#include <csignal>
#include <cstring>
#include <iostream>
#include <stdexcept>

#include "ConfigParser.hpp"
#include "lib/utils/Bzero.hpp"
#include "socket/ClientSocket.hpp"
#include "socket/ServerSocket.hpp"

Webserv::Webserv() : epoll_fd_(-1) {
}

Webserv::~Webserv() {
  ClearResources();
}

Webserv::Webserv(const std::string& config_file) {
  signal(SIGPIPE, SIG_IGN);  // avoid client disconnect crashes

  ConfigParser config_parser;
  config_parser.LoadFile(config_file);
  config_parser.Parse();
  const std::vector<ServerConfig>& configs = config_parser.GetServerConfigs();
  InitServersFromConfigs(configs);

  epoll_fd_ = epoll_create1(0);
  if (epoll_fd_ == -1) {
    throw std::runtime_error("epoll_create1() failed. " +
                             std::string(strerror(errno)));
  }

  try {
    for (std::map<unsigned short, ServerConfig>::const_iterator it =
             port_to_server_configs_.begin();
         it != port_to_server_configs_.end(); ++it) {
      unsigned short port = it->first;
      const ServerConfig& config = it->second;

      int server_fd = socket(PF_INET, SOCK_STREAM, 0);
      if (server_fd == -1) {
        throw std::runtime_error("socket() failed. " +
                                 std::string(strerror(errno)));
      }

      int opt = 1;
      if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) ==
          -1) {
        throw std::runtime_error("setsockopt() failed. " +
                                 std::string(strerror(errno)));
      }

      sockaddr_in server_addr;
      lib::utils::Bzero(&server_addr, sizeof(server_addr));
      server_addr.sin_family = AF_INET;
      server_addr.sin_addr.s_addr = INADDR_ANY;
      server_addr.sin_port = htons(port);

      if (bind(server_fd, (sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        throw std::runtime_error("bind() failed. " +
                                 std::string(strerror(errno)));
      }

      if (listen(server_fd, SOMAXCONN) == -1) {
        throw std::runtime_error("listen() failed. " +
                                 std::string(strerror(errno)));
      }

      ServerSocket* server_socket = new ServerSocket(server_fd, config);
      sockets_[server_fd] = server_socket;

      epoll_event ev;
      ev.events = EPOLLIN;
      ev.data.ptr = server_socket;
      if (epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, server_fd, &ev) == -1) {
        throw std::runtime_error("epoll_ctl() failed. " +
                                 std::string(strerror(errno)));
      }
    }
  } catch (...) {
    ClearResources();
    throw;
  }
}

void Webserv::Run() {
  epoll_event events[kMaxEvents];
  while (true) {
    int nfds = epoll_wait(epoll_fd_, events, kMaxEvents, -1);
    if (nfds == -1) {
      std::cerr << "epoll_wait() failed. " << strerror(errno) << std::endl;
      continue;
    }

    for (int i = 0; i < nfds; ++i) {
      ASocket* socket = static_cast<ASocket*>(events[i].data.ptr);
      SocketResult result = socket->HandleEvent(epoll_fd_, events[i].events);

      if (result.new_socket) {
        sockets_[result.new_socket->GetFd()] = result.new_socket;
      }

      if (result.remove_socket) {
        sockets_.erase(socket->GetFd());
        delete socket;
      }
    }
  }
}

void Webserv::InitServersFromConfigs(const std::vector<ServerConfig>& configs) {
  port_to_server_configs_.clear();
  for (std::vector<ServerConfig>::const_iterator server = configs.begin();
       server != configs.end(); ++server) {
    const unsigned short& port = server->GetPort();
    if (port_to_server_configs_.find(port) != port_to_server_configs_.end()) {
      std::cerr << "Warning: Multiple server blocks for port " << port
                << ", using first one only" << std::endl;
      continue;
    }
    port_to_server_configs_[port] = *server;
  }
}

const std::map<unsigned short, ServerConfig>& Webserv::GetPortConfigs() const {
  return port_to_server_configs_;
}

const ServerConfig* Webserv::FindServerConfigByPort(
    const unsigned short& port) const {
  std::map<unsigned short, ServerConfig>::const_iterator it =
      port_to_server_configs_.find(port);
  if (it != port_to_server_configs_.end()) {
    return &it->second;
  }
  return NULL;
}

void Webserv::ClearResources() {
  for (std::map<int, ASocket*>::iterator it = sockets_.begin();
       it != sockets_.end(); ++it) {
    delete it->second;
  }
  sockets_.clear();
  if (epoll_fd_ != -1) {
    close(epoll_fd_);
    epoll_fd_ = -1;
  }
}
