#ifndef HTTPREQUEST_HPP_
#define HTTPREQUEST_HPP_
#include <cstddef>  // for std::ptrdiff_t
#include <cstring>  // for std::tolower and std::strncmp
#include <map>
#include <string>

#include "lib/exception/ResponseStatusException.hpp"
#include "lib/http/Method.hpp"
#include "lib/http/Status.hpp"
#include "lib/parser/StreamParser.hpp"
#include "lib/type/Optional.hpp"

typedef std::map<std::string, std::string> Dict;

class HttpRequest : public lib::parser::StreamParser {
 private:
  lib::http::Method method_;
  std::string uri_;
  std::string query_string_;
  std::string host_name_;
  unsigned short host_port_;
  std::string version_;
  Dict headers_;
  std::string body_;
  long content_length_;
  std::ptrdiff_t next_chunk_size_;  // -1: waiting for chunk size line
  bool keep_alive_;
  std::string client_ip_;
  size_t max_body_size_; // numeric_limits<size_t>::max() == unlimited

  const char* ParseHeader(const char* req);

  bool ValidateAndSkipCRLF(size_t& pos) {
    if (pos + 1 >= buffer_.size()) return false;
    if (buffer_[pos] != '\r' || buffer_[pos + 1] != '\n') {
      throw lib::exception::ResponseStatusException(lib::http::kBadRequest);
    }
    pos += 2;
    return true;
  }

  void StoreHeader(const std::string& raw_key, const std::string& value);
  void ValidateAndExtractHost();
  void ValidateBodyHeaders();
  void ParseContentLength(const std::string& s);
  void ParseTransferEncoding(const std::string& s);
  void ParseConnectionDirective();
  // AdvanceBody helpers
  bool AdvanceContentLengthBody();
  bool AdvanceChunkedBody();
  bool ParseChunkSize(size_t& pos, size_t& chunk_size);
  bool ValidateFinalCRLF(size_t& pos);
  bool AppendChunkData(size_t& pos, size_t chunk_size);
  void OnInternalStateError();
  void OnExtraDataAfterDone();
  virtual bool IsStrictCrlf() const;

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
  static const unsigned short kDefaultPort;

  HttpRequest();
  HttpRequest(const HttpRequest& src);
  HttpRequest& operator=(const HttpRequest& src);
  ~HttpRequest();

  bool AdvanceHeader();
  bool AdvanceBody();
  const char* ConsumeMethod(const char* req);
  const char* ConsumeVersion(const char* req);
  const char* ConsumeUri(const char* req);
  const char* ConsumeQuery(const char* req, std::size_t& len);
  const char* ConsumeHeader(const char* req);
  lib::http::Method GetMethod() const;
  void SetMethod(lib::http::Method method);  // for test purposes
  const std::string& GetUri() const;
  void SetUri(const std::string& uri);  // for test purposes
  const std::string& GetHostName() const;
  const unsigned short& GetHostPort() const;
  const std::string& GetVersion() const;
  const Dict& GetHeader() const;
  lib::type::Optional<std::string> GetHeader(const std::string& key) const;
  const std::string& GetQuery() const;
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
    return state_ == kDone;
  }  // for test purposes

  const std::string& GetClientIp() const {
    return client_ip_;
  }

  void SetClientIp(const std::string& ip) {
    client_ip_ = ip;
  }

  State GetState() const {  // IsDone だけじゃ足りないとき用
    return state_;
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

  void SetStateForTest(State p) {
    state_ = p;
  }

  void SetMaxBodySize(size_t bytes) {
    max_body_size_ = bytes;
  }
};

#endif  // HTTPREQUEST_HPP_
