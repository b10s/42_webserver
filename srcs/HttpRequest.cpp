#include "HttpRequest.hpp"

// 外部定義（初期化子なし）
const size_t HttpRequest::kMaxHeaderSize;
const size_t HttpRequest::kMaxPayloadSize;
const size_t HttpRequest::kMaxUriSize;

HttpRequest::HttpRequest()
    : progress(HEADER),
      buffer_(),
      method_(UNKNOWN_METHOD),
      uri_(),
      hostName_(),
      hostPort_("8080"),
      version_(),
      headers_(),
      body_(),
      contentLength_(-1),  // default: unknown length, chunked possible
      keepAlive(false) {
}

HttpRequest::HttpRequest(const HttpRequest& src) {
  *this = src;
}

HttpRequest& HttpRequest::operator=(const HttpRequest& src) {
  if (this != &src) {
    buffer_ = src.buffer_;
    method_ = src.method_;
    uri_ = src.uri_;
    hostName_ = src.hostName_;
    hostPort_ = src.hostPort_;
    version_ = src.version_;
    headers_ = src.headers_;
    body_ = src.body_;
    contentLength_ = src.contentLength_;
    keepAlive = src.keepAlive;
    progress = src.progress;
  }
  return *this;
}

HttpRequest::~HttpRequest() {
}

RequestMethod HttpRequest::getMethod() const {
  return method_;
}

const std::string& HttpRequest::getUri() const {
  return uri_;
}

const dict& HttpRequest::getQuery() const {
  return query_;
}

const std::string& HttpRequest::getHostName() const {
  return hostName_;
}

const std::string& HttpRequest::getHostPort() const {
  return hostPort_;
}

const std::string& HttpRequest::getVersion() const {
  return version_;
}

const dict& HttpRequest::getHeader() const {
  return headers_;
}

const std::string& HttpRequest::getHeader(const std::string& key) const {
  dict::const_iterator it = headers_.find(key);
  if (it != headers_.end()) {
    return it->second;
  } else {
    static const std::string e;
    return e;
  }
}

const std::string& HttpRequest::getBody() const {
  return body_;
}
