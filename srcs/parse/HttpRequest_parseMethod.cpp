#include "HttpRequest.hpp"

const char* HttpRequest::parseMethod(const char* req) {
  switch (req[0]) {
    case 'G':
      if (std::strncmp(req, "GET ", 4) == 0) {
        method_ = GET;
        return req + 4;
      }
      break;
    case 'H':
      if (std::strncmp(req, "HEAD ", 5) == 0) {
        method_ = HEAD;
        return req + 5;
      }
      break;
    case 'P':
      if (std::strncmp(req, "POST ", 5) == 0) {
        method_ = POST;
        return req + 5;
      }
      break;
    case 'D':
      if (std::strncmp(req, "DELETE ", 7) == 0) {
        method_ = DELETE_;
        return req + 7;
      }
      break;
    default:
      break;
  }
  throw http::responseStatusException(NOT_IMPLEMENTED);
}

