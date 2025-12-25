#include "HttpRequest.hpp"
#include "lib/exception/ResponseStatusException.hpp"
#include "lib/http/Status.hpp"

const char* HttpRequest::ConsumeMethod(const char* req) {
  switch (req[0]) {
    case 'G':
      if (std::strncmp(req, "GET ", 4) == 0) {
        method_ = lib::http::kGet;
        return req + 4;
      }
      break;
    case 'H':
      if (std::strncmp(req, "HEAD ", 5) == 0) {
        method_ = lib::http::kHead;
        return req + 5;
      }
      break;
    case 'P':
      if (std::strncmp(req, "POST ", 5) == 0) {
        method_ = lib::http::kPost;
        return req + 5;
      }
      break;
    case 'D':
      if (std::strncmp(req, "DELETE ", 7) == 0) {
        method_ = lib::http::kDelete;
        return req + 7;
      }
      break;
    default:
      break;
  }
  throw lib::exception::ResponseStatusException(lib::http::kNotImplemented);
}
