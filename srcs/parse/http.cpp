#include "HttpRequest.hpp"

namespace http {
std::string statusToString(HttpStatus status) {
  switch (status) {
    case OK:
      return "OK";
    case CREATED:
      return "Created";
    case ACCEPTED:
      return "Accepted";
    case NO_CONTENT:
      return "No Content";
    case RESET_CONTENT:
      return "Reset Content";
    case TEMPORARY_REDIRECT:
      return "Temporary Redirect";
    case BAD_REQUEST:
      return "Bad Request";
    case UNAUTHORIZED:
      return "Unauthorized";
    case FORBIDDEN:
      return "Forbidden";
    case NOT_FOUND:
      return "Not Found";
    case METHOD_NOT_ALLOWED:
      return "Method Not Allowed";
    case URI_TOO_LONG:
      return "URI Too Long";
    case INTERNAL_SERVER_ERROR:
      return "Internal Server Error";
    case NOT_IMPLEMENTED:
      return "Not Implemented";
    case BAD_GATEWAY:
      return "Bad Gateway";
    default:
      return "I'm a teapot";
  }
}

responseStatusException::responseStatusException(HttpStatus status)
    : std::runtime_error(statusToString(status)), status_(status) {
}

HttpStatus responseStatusException::getStatus() const {
  return this->status_;
}

std::string methodToString(RequestMethod method) {
  switch (method) {
    case GET:
      return "GET";
    case HEAD:
      return "HEAD";
    case POST:
      return "POST";
    case DELETE:
      return "DELETE";
    case UNKNOWN_METHOD:
      return "UNKNOWN_METHOD";
    default:
      return "NONE";
  }
}
}  // namespace http
