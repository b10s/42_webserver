#include "HttpRequest.hpp"

HttpRequest::HttpRequest(const HttpRequest &src) { *this = src; }
HttpRequest &HttpRequest::operator=(const HttpRequest &src)
{
  if (this != &src)
  {
    this->buffer_ = src.buffer_;
    this->method_ = src.method_;
    this->uri_ = src.uri_;
    this->query_ = src.query_;
    this->hostName_ = src.hostName_;
    this->hostPort_ = src.hostPort_;
    this->version_ = src.version_;
    this->headers_ = src.headers_;
    this->body_ = src.body_;
    this->contentLength_ = src.contentLength_;
    this->keepAlive = src.keepAlive;
    this->progress = src.progress;
  }
  return *this;
}
HttpRequest::~HttpRequest() {}

const size_t HttpRequest::kMaxHeaderSize = 8192;
const size_t HttpRequest::kMaxPayloadSize = 16384;
const size_t HttpRequest::kMaxUriSize = 1024;

HttpMethod HttpRequest::getMethod() const { return this->method_; }
const std::string &HttpRequest::getUri() const { return this->uri_; }
const dict &HttpRequest::getQuery() const { return this->query_; }
const std::string &HttpRequest::getQuery(const std::string &key) const
{
  return this->query_.at(key);
}
const std::string &HttpRequest::getQueryAsStr() const
{
  static std::string query;
  query.clear();
  for (dict::const_iterator it = this->query_.begin(); it != this->query_.end();
       it++)
  {
    query += it->first + "=" + it->second + "&";
  }
  if (!query.empty())
  {
    query.erase(query.end() - 1);
  }
  return query;
}
const std::string &HttpRequest::getHostName() const { return this->hostName_; }
const std::string &HttpRequest::getHostPort() const { return this->hostPort_; }
const std::string &HttpRequest::getVersion() const { return this->version_; }
const dict &HttpRequest::getHeader() const { return this->headers_; }
const std::string &HttpRequest::getHeader(const std::string &key) const
{
  dict::const_iterator it = this->headers_.find(key);
  if (it != this->headers_.end())
  {
    return it->second;
  }
  else
  {
    static const std::string e;
    return e;
  }
}
const std::string &HttpRequest::getBody() const { return this->body_; }
