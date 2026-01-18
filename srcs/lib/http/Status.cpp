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
    case kFound:
      return "Found";
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
    case kLengthRequired:
      return "Length Required";
    case kPayloadTooLarge:
      return "Payload Too Large";
    case kUriTooLong:
      return "URI Too Long";
    case kRequestHeaderFieldsTooLarge:
      return "Request Header Fields Too Large";
    case kInternalServerError:
      return "Internal Server Error";
    case kNotImplemented:
      return "Not Implemented";
    case kBadGateway:
      return "Bad Gateway";
    default:
      return "I'm a teapot";
  }
}

}  // namespace http
}  // namespace lib
