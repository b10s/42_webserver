#include "HttpRequest.hpp"

namespace http {
std::string StatusToString(HttpStatus status) {
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
    case REQUEST_HEADER_FIELDS_TOO_LARGE:
      return "Request Header Fields Too Large";
    case LENGTH_REQUIRED:
      return "Length Required";
    default:
      return "I'm a teapot";
  }
}

ResponseStatusException::ResponseStatusException(HttpStatus status)
    : std::runtime_error(StatusToString(status)), status_(status) {
}

HttpStatus ResponseStatusException::GetStatus() const {
  return status_;
}

std::string MethodToString(RequestMethod method) {
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
