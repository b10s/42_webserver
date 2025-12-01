#ifndef HTTPREQUEST_HPP_
#define HTTPREQUEST_HPP_
#include <cstddef>  // for std::ptrdiff_t
#include <cstring>  // for std::tolower and std::strncmp
#include <limits>  // can't use SIZE_MAX in C++98 so use std::numeric_limits instead
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
  // Progress progress_;    // progress is initially kHeader
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
  size_t buffer_read_pos_;  // the position in buffer_ which has been read
  std::ptrdiff_t next_chunk_size_;  // -1: waiting for chunk size line
  bool keep_alive_;

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
  // AdvanceBodyParsing helpers
  bool AdvanceContentLengthBody();
  bool AdvanceChunkedBody();
  bool ParseChunkSize(size_t& pos, size_t& chunk_size);
  bool ValidateFinalCRLF(size_t& pos);
  bool AppendChunkData(size_t& pos, size_t chunk_size);

 public:
  enum Progress {
    kHeader = 0,  // initial state, reading header
    kBody,        // reading body
    kDone         // finished parsing request
  };

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

  HttpRequest();
  HttpRequest(const HttpRequest& src);
  HttpRequest& operator=(const HttpRequest& src);
  ~HttpRequest();

  void ParseRequest(const char* payload);
  bool AdvanceBodyParsing();
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

  const std::string& GetBufferForTest() const {  // for test purposes
    return buffer_;
  }

  bool IsKeepAlive() const {
    return keep_alive_;
  }

  bool IsDone() const {
    return progress_ == kDone;
  }  // for test purposes

  Progress GetProgress() const {  // IsDone だけじゃ足りないとき用
    return progress_;
  }

  void SetBufferForTest(const std::string& s) {
    buffer_ = s;
  }

  void AppendToBufferForTest(const std::string& s) {
    buffer_.append(s);
  }

  void SetContentLengthForTest(long len) {
    content_length_ = len;
  }

  void SetProgressForTest(Progress p) {
    progress_ = p;
  }

 private:
  Progress progress_;  // progress is initially kHeader
};

#endif  // HTTPREQUEST_HPP_
