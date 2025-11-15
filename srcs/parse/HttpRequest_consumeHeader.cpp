#include <cctype>   // for std::tolower
#include <cstdlib>  // for atoi
#include <sstream>

#include "HttpRequest.hpp"

// ============== utils ===============
bool HttpRequest::isCRLF(const char* p) const {
  return p != NULL && p[0] == '\r' && p[1] == '\n';
}

std::string HttpRequest::toLowerAscii(const std::string& s) {
  std::string result = s;
  for (size_t i = 0; i < result.size(); ++i) {
    result[i] =
        static_cast<char>(std::tolower(static_cast<unsigned char>(result[i])));
  }
  return result;
}

void HttpRequest::bumpLenOrThrow(size_t& total, size_t inc) const {
  if (inc > kMaxHeaderSize - total) {
    throw http::ResponseStatusException(REQUEST_HEADER_FIELDS_TOO_LARGE);
  }
  total += inc;
}

// we allow only single space after ":" and require CRLF at end
// OWS (optional whitespace) is not supported for simplicity
const char* HttpRequest::readHeaderLine(const char* req, std::string& key,
                                        std::string& value, size_t& totalLen) {
  size_t i = 0;
  while (req[i] && req[i] != ':') {
    bumpLenOrThrow(totalLen, 1);
    ++i;
  }
  if (req[i] == '\0' || req[i] != ':' ||
      req[i + 1] != ' ') {  // must be ": ", not ":" or end of string
    throw http::ResponseStatusException(BAD_REQUEST);
  }
  key.assign(req, i);
  // ": " を飛ばす
  i += 2;
  bumpLenOrThrow(totalLen, 2);
  req += i;
  // 2) 値は CR まで
  size_t vlen = 0;
  while (req[vlen] && req[vlen] != '\r') {
    bumpLenOrThrow(totalLen, 1);
    ++vlen;
  }
  if (!isCRLF(req + vlen)) {
    throw http::ResponseStatusException(BAD_REQUEST);
  }
  value.assign(req, vlen);      // value can be empty
  bumpLenOrThrow(totalLen, 2);  // skip CRLF
  return req + vlen + 2;
}

// Store header key in lowercase. We are not keeping original case for simplicity.
void HttpRequest::storeHeader(const std::string& rawKey,
                              const std::string& value) {
  std::string k = toLowerAscii(rawKey);
  this->headers_[k] = value;
}

void HttpRequest::validateAndExtractHost() {
  std::string hostValue;
  if (headers_.count("host"))
    hostValue = headers_["host"];
  else
    throw http::ResponseStatusException(BAD_REQUEST);

  if (hostValue.empty()) throw http::ResponseStatusException(BAD_REQUEST);
  size_t i = 0;
  while (i < hostValue.size() && hostValue[i] != ':') ++i;
  if (i == 0) throw http::ResponseStatusException(BAD_REQUEST);
  this->hostName_ = hostValue.substr(0, i);
  if (i == hostValue.size()) {
    this->hostPort_ = kDefaultPort;
  } else {
    this->hostPort_ = hostValue.substr(i + 1);
  }
}

// This line checks for both 'Transfer-Encoding' and 'transfer-encoding',
// but since headers are normalized to lowercase, only 'transfer-encoding'
// should be accessed?
void HttpRequest::validateBodyHeaders() {
  bool hasCL = headers_.count("content-length");
  bool hasTE = headers_.count("transfer-encoding");
  if (hasCL && hasTE) {
    throw http::ResponseStatusException(BAD_REQUEST);
  }
  if (hasCL) {
    const std::string& s = headers_["content-length"];
    parseContentLength(s);
  } else if (hasTE) {
    const std::string& s = headers_["transfer-encoding"];
    parseTransferEncoding(s);
  } else {
    if (method_ == POST) {
      throw http::ResponseStatusException(LENGTH_REQUIRED);
    }
    contentLength_ = 0;
  }
}

void HttpRequest::parseContentLength(const std::string& s) {
  std::stringstream ss(s);
  ss >> contentLength_;
  if (ss.fail() || contentLength_ < 0 ||
      static_cast<size_t>(contentLength_) > kMaxPayloadSize) {
    throw http::ResponseStatusException(BAD_REQUEST);
  }
}

// we allow only "chunked" for simplicity
void HttpRequest::parseTransferEncoding(const std::string& s) {
  if (s == "chunked") {
    contentLength_ = -1;
  } else {
    throw http::ResponseStatusException(NOT_IMPLEMENTED);
  }
}

// TODO: Since headers are normalized to lowercase, checking for both
// 'Connection' and 'connection' is redundant? Only 'connection' should be
// checked?
void HttpRequest::parseConnectionDirective() {
  const std::string key = "connection";
  if (headers_.count(key)) {
    const std::string& v = headers_[key];
    if (v == "close")
      keepAlive = false;
    else if (v == "keep-alive")
      keepAlive = true;
    else
      throw http::ResponseStatusException(BAD_REQUEST);
    return;
  }
  // HTTP/1.1 default is keep-alive
  keepAlive = (version_ == "HTTP/1.1");
}

const char* HttpRequest::consumeHeader(const char* req) {
  size_t totalLen = 0;
  while (*req && !isCRLF(req)) {
    std::string key, value;
    req = readHeaderLine(req, key, value, totalLen);
    storeHeader(key, value);
  }
  if (!isCRLF(req)) {
    throw http::ResponseStatusException(BAD_REQUEST);
  }
  bumpLenOrThrow(totalLen, 2);
  req += 2;  // skip CRLF
  validateAndExtractHost();
  validateBodyHeaders();
  parseConnectionDirective();
  return req;
}
