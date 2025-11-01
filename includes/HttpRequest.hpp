#ifndef HTTPREQUEST_HPP_
#define HTTPREQUEST_HPP_
#include "enums.hpp"
#include <string>

class HttpRequest
{
private:
  enum Progress
  {
    HEADER = 0, // initial state, reading header
    BODY, // reading body
    DONE // finished parsing request
  } progress = HEADER;
  std::string buffer_;
  static const size_t kMaxHeaderSize;
  static const size_t kMaxUriSize;
  RequestMethod method_;
  std::string uri_;
  std::string hostName_;
  std::string hostPort_;
  std::string version_;
  std::string body_;
  long contentLength_;

  bool consumeHeader(); // returns false if more data needed
  bool consumeBody();
  static size_t is_end_of_header(const std::string &payload);
  const char *parseMethod(const char *req);
  const char *parseUri(const char *req);
  const char *parseVersion(const char *req);
  const char *parseHeader(const char *req);

public:
  static const size_t kMaxPayloadSize;
  bool keepAlive;

  HttpRequest();
  HttpRequest(const RequestMethod &src);
  HttpRequest &operator=(const RequestMethod &src);
  ~HttpRequest();

  void parseRequest(const char *payload);
  RequestMethod getMethod() const;
  const std::string &getUri() const;
  const dict &getQuery() const;
  const std::string &getQuery(const std::string &key) const;
  const std::string &getQueryAsStr() const;
  const std::string &getHostName() const;
  const std::string &getHostPort() const;
  const std::string &getVersion() const;
  const dict &getHeader() const;
  const std::string &getHeader(const std::string &key) const;
  const std::string &getBody() const;
};
