#ifndef ENUMS_HPP_
#define ENUMS_HPP_

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

enum RequestMethod { kNone, kGet, kHead, kPost, kDelete, kUnknownMethod };

#endif  // ENUMS_HPP_
