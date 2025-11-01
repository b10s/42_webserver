#include "HttpRequest.hpp"

// HTTP tokens must be visible ASCII
namespace {
  bool isVisibleAscii(char c) {
    return c >= '!' && c <= '~'; 
  }
}

const char *HttpRequest::parseVersion(const char *req)
{
  std::size_t len = 0;
  while (req[len] && req[len] != '\r')
  {
    if (!isVisibleAscii(req[len]))
    {
      throw http::responseStatusException(BAD_REQUEST);
    }
    ++len;
  }

  if (req[len] != '\r' || req[len + 1] != '\n')
  {
    throw http::responseStatusException(BAD_REQUEST); // Missing CRLF
  }
  version_.assign(req, len);
  if (version_ != "HTTP/1.1" && version_ != "HTTP/1.0")
  {
    throw http::responseStatusException(BAD_REQUEST);
  }
  return req + len + 2;   // Advance past "\r\n"
}
