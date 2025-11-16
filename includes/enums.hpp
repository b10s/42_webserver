#ifndef ENUMS_HPP
#define ENUMS_HPP

enum TokenType {
  kTokenUnknown,
  kTokenListen,
  kTokenServerName,
  kTokenMaxBody,
  kTokenErrorPage,
  kTokenLocation,
  // Location directives
  kTokenAllowMethods,
  kTokenRoot,
  kTokenAutoindex,
  kTokenIndex,
  kTokenExtension,
  kTokenUploadPath,
  kTokenRedirect,
  kTokenCgiPath
};

enum HttpStatus {
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
  kUriTooLong = 414,
  kRequestHeaderFieldsTooLarge = 431,
  kInternalServerError = 500,
  kNotImplemented = 501,
  kBadGateway = 502
};

// enum HttpStatus {
//   kOk = 200,
//   CREATED = 201,
//   ACCEPTED = 202,
//   NO_CONTENT = 204,
//   RESET_CONTENT = 205,
//   TEMPORARY_REDIRECT = 307,
//   BAD_REQUEST = 400,
//   UNAUTHORIZED = 401,
//   FORBIDDEN = 403,
//   NOT_FOUND = 404,
//   METHOD_NOT_ALLOWED = 405,
//   LENGTH_REQUIRED = 411,
//   URI_TOO_LONG = 414,
//   REQUEST_HEADER_FIELDS_TOO_LARGE = 431,
//   INTERNAL_SERVER_ERROR = 500,
//   NOT_IMPLEMENTED = 501,
//   BAD_GATEWAY = 502
// };

enum RequestMethod {
  kNone,
  kGet,
  kHead,
  kPost,
  kDelete,
  kUnknownMethod
};
// enum RequestMethod { NONE, GET, HEAD, POST, DELETE, UNKNOWN_METHOD };

#endif
