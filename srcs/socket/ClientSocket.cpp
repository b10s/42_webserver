#include "socket/ClientSocket.hpp"

#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>

#include <iostream>

#include "RequestHandler.hpp"

ClientSocket::ClientSocket(int fd, const ServerConfig& config,
                           const std::string& client_ip)
    : ASocket(fd), config_(config) {
  request_.SetClientIp(client_ip);
}

ClientSocket::~ClientSocket() {
}

SocketResult ClientSocket::HandleEvent(int epoll_fd, uint32_t events) {
  SocketResult result;
  try {
    if (events & EPOLLIN) {
      HandleEpollIn(epoll_fd);
    }
    if (events & EPOLLOUT) {
      HandleEpollOut();
    }
  } catch (const lib::exception::ConnectionClosed& e) {
    result.remove_socket = true;
    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd_, NULL);
  } catch (const std::exception& e) {
    std::cerr << "ClientSocket error: " << e.what() << std::endl;
    result.remove_socket = true;
    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd_, NULL);
  }
  return result;
}

void ClientSocket::HandleEpollIn(int epoll_fd) {
  char buffer[kBufferSize];
  ssize_t bytes_received = recv(fd_, buffer, sizeof(buffer), 0);

  if (bytes_received <= 0) {
    throw lib::exception::ConnectionClosed();
  }

  request_.Parse(buffer, bytes_received);
  if (request_.IsDone()) {
    RequestHandler handler(config_, request_);
    response_ = handler.Run();
    output_buffer_ = response_.ToHttpString();

    epoll_event ev;
    ev.events = EPOLLOUT;
    ev.data.ptr = this;
    epoll_ctl(epoll_fd, EPOLL_CTL_MOD, fd_, &ev);
  }
}

void ClientSocket::HandleEpollOut() {
  if (output_buffer_.empty()) return;

  ssize_t bytes_sent =
      send(fd_, output_buffer_.c_str(), output_buffer_.length(), 0);

  if (bytes_sent == -1) {
    throw lib::exception::ConnectionClosed();
  }

  if (static_cast<size_t>(bytes_sent) < output_buffer_.length()) {
    output_buffer_ = output_buffer_.substr(bytes_sent);
  } else {
    output_buffer_.clear();
    throw lib::exception::ConnectionClosed();
  }
}
