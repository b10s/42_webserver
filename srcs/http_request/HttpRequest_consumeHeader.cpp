#include <cctype>   // for std::tolower
#include <cstdlib>  // for atoi

#include "HttpRequest.hpp"
#include "lib/exception/ResponseStatusException.hpp"
#include "lib/http/Method.hpp"
#include "lib/http/Status.hpp"
#include "lib/utils/string_utils.hpp"

// ============== utils ===============
bool HttpRequest::IsCRLF(const char* p) const {
  return p != NULL && p[0] == '\r' && p[1] == '\n';
}

void HttpRequest::BumpLenOrThrow(size_t& total, size_t inc) const {
  if (inc > kMaxHeaderSize - total) {
    throw lib::exception::ResponseStatusException(
        lib::http::kRequestHeaderFieldsTooLarge);
  }
  total += inc;
}

// we allow only single space after ":" and require CRLF at end
// OWS (optional whitespace) is not supported for simplicity
const char* HttpRequest::ReadHeaderLine(const char* req, std::string& key,
                                        std::string& value, size_t& total_len) {
  size_t i = 0;
  while (req[i] && req[i] != ':') {
    if (!http::IsValidHeaderChar(req[i])) {
      throw lib::exception::ResponseStatusException(lib::http::kBadRequest);
    }
    BumpLenOrThrow(total_len, 1);
    ++i;
  }
  if (req[i] == '\0' || req[i] != ':' ||
      req[i + 1] != ' ') {  // must be ": ", not ":" or end of string
    throw lib::exception::ResponseStatusException(lib::http::kBadRequest);
  }
  key.assign(req, i);
  // ": " を飛ばす
  i += 2;
  BumpLenOrThrow(total_len, 2);
  req += i;
  size_t vlen = 0;
  while (req[vlen] && req[vlen] != '\r') {
    if (!http::IsValidHeaderChar(req[vlen])) {
      throw lib::exception::ResponseStatusException(lib::http::kBadRequest);
    }
    BumpLenOrThrow(total_len, 1);
    ++vlen;
  }
  if (!IsCRLF(req + vlen)) {
    throw lib::exception::ResponseStatusException(lib::http::kBadRequest);
  }
  value.assign(req, vlen);       // value can be empty
  BumpLenOrThrow(total_len, 2);  // skip CRLF
  return req + vlen + 2;
}

// Store header key in lowercase.
// We are not keeping original case for simplicity.
void HttpRequest::StoreHeader(const std::string& raw_key,
                              const std::string& value) {
  std::string k = lib::utils::ToLowerAscii(raw_key);
  // Reject all duplicate headers
  if (headers_.count(k) > 0) {
    throw lib::exception::ResponseStatusException(lib::http::kBadRequest);
  }
  headers_[k] = value;
}

void HttpRequest::ValidateAndExtractHost() {
  Dict::const_iterator it = headers_.find("host");
  if (it == headers_.end())
    throw lib::exception::ResponseStatusException(lib::http::kBadRequest);
  const std::string& host_value = it->second;
  if (host_value.empty())
    throw lib::exception::ResponseStatusException(lib::http::kBadRequest);
  size_t i = 0;
  while (i < host_value.size() && host_value[i] != ':') ++i;
  if (i == 0)
    throw lib::exception::ResponseStatusException(lib::http::kBadRequest);
  host_name_ = host_value.substr(0, i);
  if (i == host_value.size()) {
    host_port_ = static_cast<unsigned short>(kDefaultPort);
  } else {
    host_port_ =
        lib::utils::StrToUnsignedShort(host_value.substr(i + 1)).Value();
  }
}

// header keys are normalized to lowercase
void HttpRequest::ValidateBodyHeaders() {
  bool has_cl = headers_.count("content-length");
  bool has_te = headers_.count("transfer-encoding");
  if (has_cl && has_te) {
    throw lib::exception::ResponseStatusException(lib::http::kBadRequest);
  }
  if (has_cl) {
    const std::string& s = headers_["content-length"];
    ParseContentLength(s);
  } else if (has_te) {
    const std::string& s = headers_["transfer-encoding"];
    ParseTransferEncoding(s);
  } else {
    if (method_ == lib::http::kPost) {
      throw lib::exception::ResponseStatusException(lib::http::kLengthRequired);
    }
    content_length_ = 0;
  }
}

void HttpRequest::ParseContentLength(const std::string& s) {
  lib::type::Optional<long> res = lib::utils::StrToLong(s);
  if (!res.HasValue()) {
    throw lib::exception::ResponseStatusException(lib::http::kBadRequest);
  }
  long len = res.Value();
  if (len < 0 || static_cast<size_t>(len) > kMaxPayloadSize) {
    throw lib::exception::ResponseStatusException(lib::http::kBadRequest);
  }
  content_length_ = len;
}

// we allow only "chunked" for simplicity
void HttpRequest::ParseTransferEncoding(const std::string& s) {
  if (s == "chunked") {
    content_length_ = -1;
  } else {
    throw lib::exception::ResponseStatusException(lib::http::kNotImplemented);
  }
}

// header keys are normalized to lowercase
void HttpRequest::ParseConnectionDirective() {
  const std::string key = "connection";
  if (headers_.count(key)) {
    const std::string& v = headers_[key];
    if (v == "close")
      keep_alive_ = false;
    else if (v == "keep-alive")
      keep_alive_ = true;
    else
      throw lib::exception::ResponseStatusException(lib::http::kBadRequest);
    return;
  }
  // HTTP/1.1 default is keep-alive
  keep_alive_ = (version_ == "HTTP/1.1");
}

const char* HttpRequest::ConsumeHeader(const char* req) {
  size_t total_len = 0;
  while (*req && !IsCRLF(req)) {
    std::string key, value;
    req = ReadHeaderLine(req, key, value, total_len);
    StoreHeader(key, value);
  }
  if (!IsCRLF(req)) {  // empty line with CRLF should follow after headers
    throw lib::exception::ResponseStatusException(lib::http::kBadRequest);
  }
  BumpLenOrThrow(total_len, 2);
  req += 2;  // skip CRLF
  ValidateAndExtractHost();
  ValidateBodyHeaders();
  ParseConnectionDirective();
  return req;
}
