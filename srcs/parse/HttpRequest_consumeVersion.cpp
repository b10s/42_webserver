#include "HttpRequest.hpp"

const char* HttpRequest::ConsumeVersion(const char* req) {
  std::size_t len = 0;
  while (req[len] && req[len] != '\r') {
    if (!http::IsVisibleAscii(req[len])) {
      throw http::ResponseStatusException(kBadRequest);
    }
    ++len;
  }

  if (req[len] != '\r' || req[len + 1] != '\n') {
    throw http::ResponseStatusException(kBadRequest);  // Missing CRLF
  }
  version_.assign(req, len);
  if (version_ != "HTTP/1.1" && version_ != "HTTP/1.0") {
    throw http::ResponseStatusException(kBadRequest);
  }
  return req + len + 2;  // Advance past "\r\n"
}
