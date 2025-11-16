#include "HttpRequest.hpp"

namespace http {
std::string StatusToString(HttpStatus status) {
  switch (status) {
    case kOk:
      return "OK";
    case kCreated:
      return "Created";
    case kAccepted:
      return "Accepted";
    case kNoContent:
      return "No Content";
    case kResetContent:
      return "Reset Content";
    case kTemporaryRedirect:
      return "Temporary Redirect";
    case kBadRequest:
      return "Bad Request";
    case kUnauthorized:
      return "Unauthorized";
    case kForbidden:
      return "Forbidden";
    case kNotFound:
      return "Not Found";
    case kMethodNotAllowed:
      return "Method Not Allowed";
    case kUriTooLong:
      return "URI Too Long";
    case kInternalServerError:
      return "Internal Server Error";
    case kNotImplemented:
      return "Not Implemented";
    case kBadGateway:
      return "Bad Gateway";
    case kRequestHeaderFieldsTooLarge:
      return "Request Header Fields Too Large";
    case kLengthRequired:
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
    case kGet:
      return "GET";
    case kHead:
      return "HEAD";
    case kPost:
      return "POST";
    case kDelete:
      return "DELETE";
    case kUnknownMethod:
      return "UNKNOWN_METHOD";
    default:
      return "kNone";
  }
}
}  // namespace http
