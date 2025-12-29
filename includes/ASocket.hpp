#ifndef ASOCKET_HPP
#define ASOCKET_HPP

#include <stdint.h>
#include <unistd.h>

class ASocket {
 public:
  ASocket(int fd);
  virtual ~ASocket();

  virtual void HandleEvent(uint32_t events) = 0;
  int GetFd() const;

 protected:
  int fd_;

 private:
  ASocket();
};

#endif
