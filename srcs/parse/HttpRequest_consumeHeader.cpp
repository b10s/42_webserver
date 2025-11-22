#include <cctype>   // for std::tolower
#include <cstdlib>  // for atoi
#include <sstream>

#include "HttpRequest.hpp"

// ============== utils ===============
bool HttpRequest::IsCRLF(const char* p) const {
  return p != NULL && p[0] == '\r' && p[1] == '\n';
}

std::string HttpRequest::ToLowerAscii(const std::string& s) {
  std::string result = s;
  for (size_t i = 0; i < result.size(); ++i) {
    result[i] =
        static_cast<char>(std::tolower(static_cast<unsigned char>(result[i])));
  }
  return result;
}

void HttpRequest::BumpLenOrThrow(size_t& total, size_t inc) const {
  if (inc > kMaxHeaderSize - total) {
    throw http::ResponseStatusException(kRequestHeaderFieldsTooLarge);
  }
  total += inc;
}

// we allow only single space after ":" and require CRLF at end
// OWS (optional whitespace) is not supported for simplicity
const char* HttpRequest::ReadHeaderLine(const char* req, std::string& key,
                                        std::string& value, size_t& total_len) {
  size_t i = 0;
  while (req[i] && req[i] != ':') {
    BumpLenOrThrow(total_len, 1);
    ++i;
  }
  if (req[i] == '\0' || req[i] != ':' ||
      req[i + 1] != ' ') {  // must be ": ", not ":" or end of string
    throw http::ResponseStatusException(kBadRequest);
  }
  key.assign(req, i);
  // ": " を飛ばす
  i += 2;
  BumpLenOrThrow(total_len, 2);
  req += i;
  size_t vlen = 0;
  while (req[vlen] && req[vlen] != '\r') {
    BumpLenOrThrow(total_len, 1);
    ++vlen;
  }
  if (!IsCRLF(req + vlen)) {
    throw http::ResponseStatusException(kBadRequest);
  }
  value.assign(req, vlen);       // value can be empty
  BumpLenOrThrow(total_len, 2);  // skip CRLF
  return req + vlen + 2;
}

// Store header key in lowercase.
// We are not keeping original case for simplicity.
void HttpRequest::StoreHeader(const std::string& raw_key,
                              const std::string& value) {
  std::string k = ToLowerAscii(raw_key);
  // Check for duplicate headers (case-insensitive)
  if (headers_.count(k) > 0) {
    throw http::ResponseStatusException(kBadRequest);
  }
  headers_[k] = value;
}

void HttpRequest::ValidateAndExtractHost() {
  std::string host_value;
  if (headers_.count("host"))
    host_value = headers_["host"];
  else
    throw http::ResponseStatusException(kBadRequest);

  if (host_value.empty()) throw http::ResponseStatusException(kBadRequest);
  size_t i = 0;
  while (i < host_value.size() && host_value[i] != ':') ++i;
  if (i == 0) throw http::ResponseStatusException(kBadRequest);
  host_name_ = host_value.substr(0, i);
  if (i == host_value.size()) {
    host_port_ = kDefaultPort;
  } else {
    host_port_ = host_value.substr(i + 1);
  }
}

// header keys are normalized to lowercase
void HttpRequest::ValidateBodyHeaders() {
  bool has_cl = headers_.count("content-length");
  bool has_te = headers_.count("transfer-encoding");
  if (has_cl && has_te) {
    throw http::ResponseStatusException(kBadRequest);
  }
  if (has_cl) {
    const std::string& s = headers_["content-length"];
    ParseContentLength(s);
  } else if (has_te) {
    const std::string& s = headers_["transfer-encoding"];
    ParseTransferEncoding(s);
  } else {
    if (method_ == kPost) {
      throw http::ResponseStatusException(kLengthRequired);
    }
    content_length_ = 0;
  }
}

void HttpRequest::ParseContentLength(const std::string& s) {
  std::stringstream ss(s);
  ss >> content_length_;
  if (ss.fail() || content_length_ < 0 ||
      static_cast<size_t>(content_length_) > kMaxPayloadSize) {
    throw http::ResponseStatusException(kBadRequest);
  }
}

// we allow only "chunked" for simplicity
void HttpRequest::ParseTransferEncoding(const std::string& s) {
  if (s == "chunked") {
    content_length_ = -1;
  } else {
    throw http::ResponseStatusException(kNotImplemented);
  }
}

// header keys are normalized to lowercase
void HttpRequest::ParseConnectionDirective() {
  const std::string key = "connection";
  if (headers_.count(key)) {
    const std::string& v = headers_[key];
    if (v == "close")
      keep_alive = false;
    else if (v == "keep-alive")
      keep_alive = true;
    else
      throw http::ResponseStatusException(kBadRequest);
    return;
  }
  // HTTP/1.1 default is keep-alive
  keep_alive = (version_ == "HTTP/1.1");
}

const char* HttpRequest::ConsumeHeader(const char* req) {
  size_t total_len = 0;
  while (*req && !IsCRLF(req)) {
    std::string key, value;
    req = ReadHeaderLine(req, key, value, total_len);
    StoreHeader(key, value);
  }
  if (!IsCRLF(req)) {
    throw http::ResponseStatusException(kBadRequest);
  }
  BumpLenOrThrow(total_len, 2);
  req += 2;  // skip CRLF
  ValidateAndExtractHost();
  ValidateBodyHeaders();
  ParseConnectionDirective();
  return req;
}
