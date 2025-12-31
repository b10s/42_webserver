#include "socket/ServerSocket.hpp"

#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <sys/socket.h>

#include <iostream>

#include "socket/ClientSocket.hpp"

ServerSocket::ServerSocket(int fd, const ServerConfig& config)
    : ASocket(fd), config_(config) {
}

ServerSocket::~ServerSocket() {
}

SocketResult ServerSocket::HandleEvent(int epoll_fd, uint32_t events) {
  SocketResult result;
  if (events & EPOLLIN) {
    sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    int client_fd = accept(fd_, (sockaddr*)&client_addr, &client_addr_len);

    if (client_fd == -1) {
      std::cerr << "Failed to accept fd:" << client_fd << std::endl;
      return result;
    }

    int flags = fcntl(client_fd, F_GETFL, 0);
    fcntl(client_fd, F_SETFL, flags | O_NONBLOCK);

    std::string client_ip = inet_ntoa(client_addr.sin_addr);
    ClientSocket* client_socket =
        new ClientSocket(client_fd, config_, client_ip);

    epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.ptr = client_socket;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &ev) == -1) {
      std::cerr << "Failed to add client socket to epoll" << std::endl;
      delete client_socket;
      return result;
    }

    std::cout << "Accepted connection from " << client_ip << std::endl;

    result.new_socket = client_socket;
  }
  return result;
}
