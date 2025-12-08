#ifndef LIB_HTTP_STATUS_HPP_
#define LIB_HTTP_STATUS_HPP_

#include <string>

namespace lib {
namespace http {

enum Status {
  kOk = 200,
  kCreated = 201,
  kAccepted = 202,
  kNoContent = 204,
  kResetContent = 205,
  kTemporaryRedirect = 307,
  kBadRequest = 400,
  kUnauthorized = 401,
  kForbidden = 403,
  kNotFound = 404,
  kMethodNotAllowed = 405,
  kLengthRequired = 411,
  kPayloadTooLarge = 413,
  kUriTooLong = 414,
  kRequestHeaderFieldsTooLarge = 431,
  kInternalServerError = 500,
  kNotImplemented = 501,
  kBadGateway = 502
};

std::string StatusToString(Status status);

}  // namespace http
}  // namespace lib

#endif  // LIB_HTTP_STATUS_HPP_
