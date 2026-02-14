#include "socket/ASocket.hpp"

#include <fcntl.h>

#include <cerrno>
#include <cstring>
#include <stdexcept>

#include "lib/exception/ResponseStatusException.hpp"
#include "lib/utils/file_utils.hpp"

ASocket::ASocket(lib::type::Fd fd)
    : fd_(fd), last_activity_time_(std::time(NULL)) {
  if (fd_.GetFd() == -1) {
    throw lib::exception::ResponseStatusException(
        lib::http::kInternalServerError);
  }
  SetNonBlocking();
}

ASocket::~ASocket() {
}

int ASocket::GetFd() const {
  return fd_.GetFd();
}

void ASocket::SetNonBlocking() const {
  if (fd_.GetFd() == -1) {
    throw lib::exception::ResponseStatusException(
        lib::http::kInternalServerError);
  }
  int flags = fcntl(fd_.GetFd(), F_GETFL, 0);
  if (flags == -1) {
    int saved_errno = errno;
    throw lib::exception::ResponseStatusException(
        lib::utils::MapErrnoToHttpStatus(saved_errno));
  }
  if (fcntl(fd_.GetFd(), F_SETFL, flags | O_NONBLOCK) == -1) {
    int saved_errno = errno;
    throw lib::exception::ResponseStatusException(
        lib::utils::MapErrnoToHttpStatus(saved_errno));
  }
}
