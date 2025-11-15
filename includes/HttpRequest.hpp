#ifndef HTTPREQUEST_HPP_
#define HTTPREQUEST_HPP_
#include <cstring>  // for std::tolower and std::strncmp
#include <map>
#include <stdexcept>
#include <string>

#include "enums.hpp"

// http
namespace http {
std::string StatusToString(HttpStatus status);
std::string MethodToString(RequestMethod method);
inline bool IsVisibleAscii(char c) {
  return c >= '!' && c <= '~';
}

class ResponseStatusException : public std::runtime_error {
 private:
  HttpStatus status_;

 public:
  ResponseStatusException(HttpStatus status);
  HttpStatus GetStatus() const;
};
}  // namespace http

typedef std::map<std::string, std::string> Dict;

class HttpRequest {
 private:
  enum Progress {
    kHeader = 0,  // initial state, reading header
    kBody,        // reading body
    kDone         // finished parsing request
  } progress_;    // progress is initially kHeader

  std::string buffer_;
  RequestMethod method_;
  std::string uri_;
  Dict query_;
  std::string host_name_;
  std::string host_port_;
  std::string version_;
  Dict headers_;
  std::string body_;
  long content_length_;

  // bool consumeHeader();  // returns false if more data needed
  bool ConsumeBody();
  static std::string::size_type FindEndOfHeader(const std::string& payload);
  const char* ParseHeader(const char* req);
  bool IsCRLF(const char* p) const;
  static std::string ToLowerAscii(const std::string& s);
  void bumpLenOrThrow(size_t& total, size_t inc) const;
  const char* ReadHeaderLine(const char* req, std::string& key,
                             std::string& value, size_t& total_len);
  void storeHeader(const std::string& raw_key, const std::string& value);
  void validateAndExtractHost();
  void validateBodyHeaders();
  void parseContentLength(const std::string& s);
  void parseTransferEncoding(const std::string& s);
  void parseConnectionDirective();

 public:
  // there is no upper limit for header count in RFCs, but we set a 8192 bytes
  // (8KB) for simplicity
  static const size_t kMaxHeaderSize = 8192;
  // the maximum size of request payload is usually 1MB or more in real servers,
  // but we set 16KB for simplicity
  static const size_t kMaxPayloadSize = 16384;
  // the maximum size of request URI is 8192 bytes (8KB) in nginx but we set
  // smaller limit (1KB) for simplicity
  static const size_t kMaxUriSize = 1024;
  static const std::string kDefaultPort;
  bool keep_alive;

  HttpRequest();
  HttpRequest(const HttpRequest& src);
  HttpRequest& operator=(const HttpRequest& src);
  ~HttpRequest();

  void parseRequest(const char* payload);
  const char* consumeMethod(const char* req);
  const char* consumeVersion(const char* req);
  const char* consumeUri(const char* req);
  const char* consumeQuery(const char* req, std::size_t& len);
  const char* consumeHeader(const char* req);
  RequestMethod getMethod() const;
  void setMethod(RequestMethod method);  // for test purposes
  const std::string& getUri() const;
  const std::string& getHostName() const;
  const std::string& getHostPort() const;
  const std::string& getVersion() const;
  const Dict& getHeader() const;
  const std::string& getHeader(const std::string& key) const;
  const Dict& getQuery() const;
  const std::string& getBody() const;

  long getContentLength() const {
    return content_length_;
  }

  bool isKeepAlive() const {
    return keep_alive;
  }

  bool isDone() const {
    return progress_ == kDone;
  }  // for test purposes
};

#endif  // HTTPREQUEST_HPP_
