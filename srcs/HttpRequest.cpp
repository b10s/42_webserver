#include "HttpRequest.hpp"

// 外部定義（初期化子なし）
const size_t HttpRequest::kMaxHeaderSize;
const size_t HttpRequest::kMaxPayloadSize;
const size_t HttpRequest::kMaxUriSize;
const std::string HttpRequest::kDefaultPort = "8080";

HttpRequest::HttpRequest()
    : progress_(kHeader),
      buffer_(),
      method_(kUnknownMethod),
      uri_(),
      host_name_(),
      host_port_("8080"),
      version_(),
      headers_(),
      body_(),
      content_length_(-1),  // default: unknown length, chunked possible
      keep_alive(false) {
}

HttpRequest::HttpRequest(const HttpRequest& src) {
  *this = src;
}

HttpRequest& HttpRequest::operator=(const HttpRequest& src) {
  if (this != &src) {
    buffer_ = src.buffer_;
    method_ = src.method_;
    uri_ = src.uri_;
    host_name_ = src.host_name_;
    host_port_ = src.host_port_;
    version_ = src.version_;
    headers_ = src.headers_;
    body_ = src.body_;
    content_length_ = src.content_length_;
    keep_alive = src.keep_alive;
    progress_ = src.progress_;
  }
  return *this;
}

HttpRequest::~HttpRequest() {
}

RequestMethod HttpRequest::GetMethod() const {
  return method_;
}

void HttpRequest::SetMethod(RequestMethod method) {
  method_ = method;
}

const std::string& HttpRequest::GetUri() const {
  return uri_;
}

const Dict& HttpRequest::GetQuery() const {
  return query_;
}

const std::string& HttpRequest::GetHostName() const {
  return host_name_;
}

const std::string& HttpRequest::GetHostPort() const {
  return host_port_;
}

const std::string& HttpRequest::GetVersion() const {
  return version_;
}

const Dict& HttpRequest::GetHeader() const {
  return headers_;
}

const std::string& HttpRequest::GetHeader(const std::string& key) const {
  Dict::const_iterator it = headers_.find(key);
  if (it == headers_.end()) {
    throw std::out_of_range("Header not found: " + key);
  }
  return it->second;
}

const std::string& HttpRequest::GetBody() const {
  return body_;
}
