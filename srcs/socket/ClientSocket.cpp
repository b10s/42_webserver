#include "socket/ClientSocket.hpp"

#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cerrno>
#include <cstring>
#include <iostream>

#include "CgiResponseParser.hpp"
#include "RequestHandler.hpp"
#include "lib/exception/ConnectionClosed.hpp"
#include "lib/type/Fd.hpp"
#include "socket/CgiSocket.hpp"

ClientSocket::ClientSocket(lib::type::Fd fd, const ServerConfig& config,
                           const std::string& client_ip)
    : ASocket(fd), config_(config) {
  req_.SetClientIp(client_ip);
}

ClientSocket::~ClientSocket() {
}

SocketResult ClientSocket::HandleEvent(int epoll_fd, uint32_t events) {
  SocketResult result;
  try {
    if (events & EPOLLIN) {
      SocketResult in_result = HandleEpollIn(epoll_fd);
      if (in_result.new_socket) {
        result.new_socket = in_result.new_socket;
      }
      if (in_result.remove_socket) {
        result.remove_socket = true;
      }
    }
    if (events & EPOLLOUT) {
      HandleEpollOut();
    }
  } catch (const lib::exception::ConnectionClosed& e) {
    result.remove_socket = true;
    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd_.GetFd(), NULL);
  } catch (const std::exception& e) {
    std::cerr << "ClientSocket error: " << e.what() << std::endl;
    result.remove_socket = true;
    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd_.GetFd(), NULL);
  }
  return result;
}

SocketResult ClientSocket::HandleEpollIn(int epoll_fd) {
  char buffer[kBufferSize];
  ssize_t bytes_received = recv(fd_.GetFd(), buffer, sizeof(buffer), 0);

  if (bytes_received <= 0) {
    throw lib::exception::ConnectionClosed();
  }

  req_.Parse(buffer, bytes_received);
  if (req_.IsDone()) {
    RequestHandler handler(config_, req_);
    ExecResult result = handler.Run();

    if (result.is_async) {
      if (result.new_socket) {
        result.new_socket->OnSetOwner(this);
      }
      SocketResult socket_result;
      socket_result.new_socket = result.new_socket;
      return socket_result;
    }

    res_ = result.response;
    write_buffer_ = res_.ToHttpString();

    epoll_event ev;
    ev.events = EPOLLOUT;
    ev.data.ptr = this;
    epoll_ctl(epoll_fd, EPOLL_CTL_MOD, fd_.GetFd(), &ev);
  }
  return SocketResult();
}

void ClientSocket::HandleEpollOut() {
  if (write_buffer_.empty()) return;

  ssize_t bytes_sent =
      send(fd_.GetFd(), write_buffer_.c_str(), write_buffer_.length(), 0);

  if (bytes_sent == -1) {
    throw lib::exception::ConnectionClosed();
  }

  if (static_cast<size_t>(bytes_sent) < write_buffer_.length()) {
    write_buffer_ = write_buffer_.substr(bytes_sent);
  } else {
    write_buffer_.clear();
    throw lib::exception::ConnectionClosed();
  }
}

void ClientSocket::OnCgiExecutionFinished(int epoll_fd,
                                          const std::string& cgi_output) {
  res_ = cgi::ParseCgiResponse(cgi_output);
  write_buffer_ = res_.ToHttpString();

  epoll_event ev;
  ev.events = EPOLLOUT;
  ev.data.ptr = this;
  epoll_ctl(epoll_fd, EPOLL_CTL_MOD, fd_.GetFd(), &ev);
}
