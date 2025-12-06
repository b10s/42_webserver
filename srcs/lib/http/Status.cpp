#include "lib/http/Status.hpp"

namespace lib {
namespace http {

std::string StatusToString(Status status) {
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
    case lib::http::kBadRequest:
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
    case lib::http::kInternalServerError:
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

}  // namespace http
}  // namespace lib
