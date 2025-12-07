#include "HttpRequest.hpp"
#include "lib/exception/ResponseStatusException.hpp"
#include "lib/http/Status.hpp"

const char* HttpRequest::ConsumeVersion(const char* req) {
  std::size_t len = 0;
  while (req[len] && req[len] != '\r') {
    if (!http::IsVisibleAscii(req[len])) {
      throw lib::exception::ResponseStatusException(lib::http::kBadRequest);
    }
    ++len;
  }

  if (req[len] != '\r' || req[len + 1] != '\n') {
    throw lib::exception::ResponseStatusException(
        lib::http::kBadRequest);  // Missing CRLF
  }
  version_.assign(req, len);
  if (version_ != "HTTP/1.1" && version_ != "HTTP/1.0") {
    throw lib::exception::ResponseStatusException(lib::http::kBadRequest);
  }
  return req + len + 2;  // Advance past "\r\n"
}
