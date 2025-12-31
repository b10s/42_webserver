#include "HttpRequest.hpp"

#include "lib/http/Method.hpp"
#include "lib/type/Optional.hpp"

// 外部定義（初期化子なし）
const size_t HttpRequest::kMaxHeaderSize;
const size_t HttpRequest::kMaxPayloadSize;
const size_t HttpRequest::kMaxUriSize;
const unsigned short HttpRequest::kDefaultPort = 8080;

HttpRequest::HttpRequest()
    : method_(lib::http::kUnknownMethod),
      uri_(),
      query_(),
      host_name_(),
      host_port_(8080),
      version_(),
      headers_(),
      body_(),
      content_length_(-1),  // default: unknown length, chunked possible
      next_chunk_size_(-1),
      keep_alive_(false),
      client_ip_() {
}

HttpRequest::HttpRequest(const HttpRequest& src)
    : lib::parser::StreamParser(src) {
  *this = src;
}

HttpRequest& HttpRequest::operator=(const HttpRequest& src) {
  if (this != &src) {
    lib::parser::StreamParser::operator=(src);
    method_ = src.method_;
    uri_ = src.uri_;
    query_ = src.query_;
    host_name_ = src.host_name_;
    host_port_ = src.host_port_;
    version_ = src.version_;
    headers_ = src.headers_;
    body_ = src.body_;
    content_length_ = src.content_length_;
    next_chunk_size_ = src.next_chunk_size_;
    keep_alive_ = src.keep_alive_;
    client_ip_ = src.client_ip_;
  }
  return *this;
}

HttpRequest::~HttpRequest() {
}

lib::http::Method HttpRequest::GetMethod() const {
  return method_;
}

void HttpRequest::SetMethod(lib::http::Method method) {
  method_ = method;
}

const std::string& HttpRequest::GetUri() const {
  return uri_;
}

void HttpRequest::SetUri(const std::string& uri) {
  uri_ = uri;
}

const Dict& HttpRequest::GetQuery() const {
  return query_;
}

const std::string& HttpRequest::GetHostName() const {
  return host_name_;
}

const unsigned short& HttpRequest::GetHostPort() const {
  return host_port_;
}

const std::string& HttpRequest::GetVersion() const {
  return version_;
}

const Dict& HttpRequest::GetHeader() const {
  return headers_;
}

lib::type::Optional<std::string> HttpRequest::GetHeader(
    const std::string& key) const {
  Dict::const_iterator it = headers_.find(key);
  if (it == headers_.end()) {
    return lib::type::Optional<std::string>();
  }
  return lib::type::Optional<std::string>(it->second);
}

const std::string& HttpRequest::GetBody() const {
  return body_;
}

void HttpRequest::OnInternalStateError() {
  throw lib::exception::ResponseStatusException(
      lib::http::kInternalServerError);
}

void HttpRequest::OnExtraDataAfterDone() {
  throw lib::exception::ResponseStatusException(lib::http::kBadRequest);
}
