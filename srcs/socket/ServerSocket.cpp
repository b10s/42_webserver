#include "socket/ServerSocket.hpp"

#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <sys/socket.h>

#include <cerrno>
#include <cstring>
#include <iostream>
#include <stdexcept>

#include "lib/type/Fd.hpp"
#include "lib/utils/Bzero.hpp"
#include "socket/ClientSocket.hpp"

ServerSocket::ServerSocket(const ServerConfig& config)
    : ASocket(), config_(config) {
  fd_.Reset(socket(PF_INET, SOCK_STREAM, 0));
  if (fd_.GetFd() == -1) {
    throw std::runtime_error("socket() failed. " +
                             std::string(strerror(errno)));
  }

  int opt = 1;
  if (setsockopt(fd_.GetFd(), SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) ==
      -1) {
    throw std::runtime_error("setsockopt() failed. " +
                             std::string(strerror(errno)));
  }

  sockaddr_in server_addr;
  lib::utils::Bzero(&server_addr, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = INADDR_ANY;
  server_addr.sin_port = htons(config_.GetPort());

  if (bind(fd_.GetFd(), (sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
    throw std::runtime_error("bind() failed. " + std::string(strerror(errno)));
  }

  if (listen(fd_.GetFd(), SOMAXCONN) == -1) {
    throw std::runtime_error("listen() failed. " + std::string(strerror(errno)));
  }
}

ServerSocket::~ServerSocket() {
}

SocketResult ServerSocket::HandleEvent(int epoll_fd, uint32_t events) {
  SocketResult result;
  if (events & EPOLLIN) {
    sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    lib::type::Fd client_fd(
        accept(fd_.GetFd(), (sockaddr*)&client_addr, &client_addr_len));

    if (client_fd.GetFd() == -1) {
      std::cerr << "Failed to accept fd:" << client_fd.GetFd() << std::endl;
      return result;
    }

    std::string client_ip = inet_ntoa(client_addr.sin_addr);
    ClientSocket* client_socket =
        new ClientSocket(client_fd, config_, client_ip);

    epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.ptr = client_socket;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_socket->GetFd(), &ev) == -1) {
      std::cerr << "Failed to add client socket to epoll" << std::endl;
      delete client_socket;
      return result;
    }

    std::cout << "Accepted connection from " << client_ip << std::endl;

    result.new_socket = client_socket;
  }
  return result;
}
