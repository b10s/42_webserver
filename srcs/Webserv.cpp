#include "Webserv.hpp"

#include <sys/epoll.h>
#include <unistd.h>
#include <ctime>


#include <cerrno>
#include <csignal>
#include <cstring>
#include <iostream>

#include "ConfigParser.hpp"
#include "lib/type/Fd.hpp"
#include "socket/ServerSocket.hpp"

Webserv::Webserv() : epoll_fd_(-1) {
}

Webserv::~Webserv() {
  ClearResources();
}

Webserv::Webserv(const std::string& config_file) {
  signal(SIGPIPE, SIG_IGN);  // avoid client disconnect crashes

  ConfigParser config_parser;
  config_parser.LoadFileOrThrowRuntime(config_file);
  config_parser.Parse();
  const std::vector<ServerConfig>& configs = config_parser.GetServerConfigs();
  InitServersFromConfigs(configs);

  epoll_fd_.Reset(epoll_create1(0));
  if (epoll_fd_.GetFd() == -1) {
    throw std::runtime_error("epoll_create1() failed. " +
                             std::string(strerror(errno)));
  }

  try {
    for (std::map<unsigned short, ServerConfig>::const_iterator it =
             port_to_server_configs_.begin();
         it != port_to_server_configs_.end(); ++it) {
      const ServerConfig& config = it->second;

      ServerSocket* server_socket = new ServerSocket(config);
      sockets_[server_socket->GetFd()] = server_socket;

      epoll_event ev;
      ev.events = EPOLLIN;
      ev.data.ptr = server_socket;
      if (epoll_ctl(epoll_fd_.GetFd(), EPOLL_CTL_ADD, server_socket->GetFd(),
                    &ev) == -1) {
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
    CheckTimeout();
    int nfds = epoll_wait(epoll_fd_.GetFd(), events, kMaxEvents, 500);
    if (nfds == -1) {
      std::cerr << "epoll_wait() failed. " << strerror(errno) << std::endl;
      continue;
    }

    for (int i = 0; i < nfds; ++i) {
      ASocket* socket = static_cast<ASocket*>(events[i].data.ptr);
      SocketResult result =
          socket->HandleEvent(epoll_fd_.GetFd(), events[i].events);

      if (result.new_socket) {
        sockets_[result.new_socket->GetFd()] = result.new_socket;
        epoll_event ev;
        ev.events = EPOLLIN;
        ev.data.ptr = result.new_socket;
        if (epoll_ctl(epoll_fd_.GetFd(), EPOLL_CTL_ADD,
                      result.new_socket->GetFd(), &ev) == -1) {
          std::cerr << "epoll_ctl() failed for new socket. " << strerror(errno)
                    << std::endl;
          sockets_.erase(result.new_socket->GetFd());
          delete result.new_socket;
        }
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

void Webserv::CheckTimeout() {
  time_t now = std::time(NULL);
  time_t threshold = now - kRequestTimeout;

  for (std::map<int, ASocket*>::iterator it = sockets_.begin();
       it != sockets_.end();) {
    if (it->second->IsTimeout(threshold)) {
      ASocket* socket = it->second;
      int fd = socket->GetFd();
      epoll_ctl(epoll_fd_.GetFd(), EPOLL_CTL_DEL, fd, NULL);
      delete socket;
      sockets_.erase(it++);
      std::cerr << "Connection timed out. fd: " << fd << std::endl;
    } else {
      ++it;
    }
  }
}

void Webserv::ClearResources() {
  for (std::map<int, ASocket*>::iterator it = sockets_.begin();
       it != sockets_.end(); ++it) {
    delete it->second;
  }
  sockets_.clear();
}
