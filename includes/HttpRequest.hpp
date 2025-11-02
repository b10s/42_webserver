#ifndef HTTPREQUEST_HPP_
#define HTTPREQUEST_HPP_
#include "enums.hpp"
#include <string>
#include <map>
#include <stdexcept>
#include <cstring> // for std::tolower and std::strncmp

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

class HttpRequest
{
private:
  enum Progress
  {
    HEADER = 0, // initial state, reading header
    BODY, // reading body
    DONE // finished parsing request
  } progress; // progress is initially HEADER
  std::string buffer_;
  static const size_t kMaxHeaderSize = 8192;
  static const size_t kMaxPayloadSize = 16384;
  static const size_t kMaxUriSize = 1024;
  RequestMethod method_;
  std::string uri_;
  std::string hostName_;
  std::string hostPort_;
  std::string version_;
  dict headers_;
  std::string body_;
  long contentLength_;

  bool consumeHeader(); // returns false if more data needed
  bool consumeBody();
  static std::string::size_type find_end_of_header(const std::string& payload);
  const char *parseUri(const char *req);
  const char *parseHeader(const char *req);
  static std::string toLowerCopy(const std::string& s);
  static void bumpLenOrThrow(size_t& acc, size_t add, size_t limit);
  static const char* parseHeaderLine(
      const char* req, size_t& accLen, std::string& outKey, std::string& outValue);
  void validateAndApplyHeaders();     // Host / CL / TE / Connection の事後処理
  void parseHostHeader(const std::string& host); // hostName_ / hostPort_ を決める

public:
  bool keepAlive;

  HttpRequest();
  HttpRequest(const HttpRequest &src);
  HttpRequest &operator=(const HttpRequest &src);
  ~HttpRequest();

  void parseRequest(const char *payload);
  const char *parseMethod(const char *req);
  const char *parseVersion(const char *req);
  RequestMethod getMethod() const;
  const std::string &getUri() const;
  const std::string &getHostName() const;
  const std::string &getHostPort() const;
  const std::string &getVersion() const;
  const dict &getHeader() const;
  const std::string &getHeader(const std::string &key) const;
  const std::string &getBody() const;
  bool isDone() const { return progress == DONE; } // for test purposes
};

#endif // HTTPREQUEST_HPP_

