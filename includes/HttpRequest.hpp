#ifndef HTTPREQUEST_HPP_
#define HTTPREQUEST_HPP_
#include <cstring>  // for std::tolower and std::strncmp
#include <map>
#include <stdexcept>
#include <string>

#include "enums.hpp"

#define DEFAULT_PORT "8080"

// http
namespace http {
std::string statusToString(HttpStatus status);
std::string methodToString(RequestMethod method);

class responseStatusException : public std::runtime_error {
 private:
  HttpStatus status_;

 public:
  responseStatusException(HttpStatus status);
  HttpStatus getStatus() const;
};
}  // namespace http

typedef std::map<std::string, std::string> dict;

class HttpRequest {
 private:
  enum Progress {
    HEADER = 0,  // initial state, reading header
    BODY,        // reading body
    DONE         // finished parsing request
  } progress;    // progress is initially HEADER

  std::string buffer_;
  RequestMethod method_;
  std::string uri_;
  std::string hostName_;
  std::string hostPort_;
  std::string version_;
  dict headers_;
  std::string body_;
  long contentLength_;

  // bool consumeHeader();  // returns false if more data needed
  bool consumeBody();
  static std::string::size_type find_end_of_header(const std::string& payload);
  const char* parseUri(const char* req);
  const char* parseHeader(const char* req);
  bool isCRLF(const char* p) const;  
  static std::string toLowerAscii(const std::string s);
  void bumpLenOrThrow(size_t& total, size_t inc) const;
  const char* readHeaderLine(const char* req, std::string& key, std::string& value, size_t& total_len);
  void storeHeader(const std::string& rawKey, const std::string& value);
  void validateAndExtractHost();
  void validateBodyHeaders();
  void parseContentLength(const std::string& s);
  void parseTransferEncoding(const std::string& s);
  void parseConnectionDirective();

 public:
  static const size_t kMaxHeaderSize = 8192;
  static const size_t kMaxPayloadSize = 16384;
  static const size_t kMaxUriSize = 1024;
  bool keepAlive;

  HttpRequest();
  HttpRequest(const HttpRequest& src);
  HttpRequest& operator=(const HttpRequest& src);
  ~HttpRequest();

  void parseRequest(const char* payload);
  const char* consumeMethod(const char* req);
  const char* consumeVersion(const char* req);
  const char* consumeHeader(const char* req);
  RequestMethod getMethod() const;
  const std::string& getUri() const;
  const std::string& getHostName() const;
  const std::string& getHostPort() const;
  const std::string& getVersion() const;
  const dict& getHeader() const;
  const std::string& getHeader(const std::string& key) const;
  const std::string& getBody() const;

  bool isDone() const {
    return progress == DONE;
  }  // for test purposes
};

#endif  // HTTPREQUEST_HPP_
