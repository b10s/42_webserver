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
  kTokenUploadPath,
  kTokenRedirect,
  kTokenCgi
};

#endif  // ENUMS_HPP_
