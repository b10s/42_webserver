#ifndef HTTPREQUEST_HPP_
#define HTTPREQUEST_HPP_
#include <cstring>  // for std::tolower and std::strncmp
#include <map>
#include <stdexcept>
#include <string>

#include "enums.hpp"
#include "lib/exception/ResponseStatusException.hpp"
#include "lib/http/Method.hpp"
#include "lib/http/Status.hpp"

// http
namespace http {
inline bool IsVisibleAscii(char c) {
  return c >= '!' && c <= '~';
}
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
  lib::http::Method method_;
  std::string uri_;
  Dict query_;
  std::string host_name_;
  std::string host_port_;
  std::string version_;
  Dict headers_;
  std::string body_;
  long content_length_;

  // bool ConsumeHeader();  // returns false if more data needed
  bool ConsumeBody();
  static std::string::size_type FindEndOfHeader(const std::string& payload);
  const char* ParseHeader(const char* req);
  bool IsCRLF(const char* p) const;
  void BumpLenOrThrow(size_t& total, size_t inc) const;
  const char* ReadHeaderLine(const char* req, std::string& key,
                             std::string& value, size_t& total_len);
  void StoreHeader(const std::string& raw_key, const std::string& value);
  void ValidateAndExtractHost();
  void ValidateBodyHeaders();
  void ParseContentLength(const std::string& s);
  void ParseTransferEncoding(const std::string& s);
  void ParseConnectionDirective();

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

  void ParseRequest(const char* payload);
  const char* ConsumeMethod(const char* req);
  const char* ConsumeVersion(const char* req);
  const char* ConsumeUri(const char* req);
  const char* ConsumeQuery(const char* req, std::size_t& len);
  const char* ConsumeHeader(const char* req);
  lib::http::Method GetMethod() const;
  void SetMethod(lib::http::Method method);  // for test purposes
  const std::string& GetUri() const;
  const std::string& GetHostName() const;
  const std::string& GetHostPort() const;
  const std::string& GetVersion() const;
  const Dict& GetHeader() const;
  const std::string& GetHeader(const std::string& key) const;
  const Dict& GetQuery() const;
  const std::string& GetBody() const;

  long GetContentLength() const {
    return content_length_;
  }

  bool IsKeepAlive() const {
    return keep_alive;
  }

  bool IsDone() const {
    return progress_ == kDone;
  }  // for test purposes
};

#endif  // HTTPREQUEST_HPP_
