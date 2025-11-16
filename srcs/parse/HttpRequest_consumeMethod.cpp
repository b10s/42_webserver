#include "HttpRequest.hpp"

const char* HttpRequest::ConsumeMethod(const char* req) {
  switch (req[0]) {
    case 'G':
      if (std::strncmp(req, "GET ", 4) == 0) {
        method_ = kGet;
        return req + 4;
      }
      break;
    case 'H':
      if (std::strncmp(req, "HEAD ", 5) == 0) {
        method_ = kHead;
        return req + 5;
      }
      break;
    case 'P':
      if (std::strncmp(req, "POST ", 5) == 0) {
        method_ = kPost;
        return req + 5;
      }
      break;
    case 'D':
      if (std::strncmp(req, "DELETE ", 7) == 0) {
        method_ = kDelete;
        return req + 7;
      }
      break;
    default:
      break;
  }
  throw http::ResponseStatusException(kNotImplemented);
}
