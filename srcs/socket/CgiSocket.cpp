#include "socket/CgiSocket.hpp"

#include <fcntl.h>
#include <unistd.h>

CgiSocket::CgiSocket(lib::type::Fd fd) : ASocket(fd) {
  // まだ CGI
  // はノンブロッキングに対応していないため、ブロッキングモードに設定する
  int flags = fcntl(fd_.GetFd(), F_GETFL, 0);
  fcntl(fd_.GetFd(), F_SETFL, flags & ~O_NONBLOCK);
}

CgiSocket::~CgiSocket() {
}

SocketResult CgiSocket::HandleEvent(int epoll_fd, uint32_t events) {
  (void)epoll_fd;
  (void)events;
  return SocketResult();
}

ssize_t CgiSocket::Send(const std::string& data) {
  return write(fd_.GetFd(), data.c_str(), data.length());
}

std::string CgiSocket::Receive() {
  std::string output;
  char buffer[4096];
  ssize_t bytes_read;

  while ((bytes_read = read(fd_.GetFd(), buffer, sizeof(buffer))) > 0) {
    output.append(buffer, bytes_read);
  }
  return output;
}
