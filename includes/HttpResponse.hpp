#ifndef HTTPRESPONSE_HPP_
#define HTTPRESPONSE_HPP_

#include <map>
#include <string>

#include "lib/http/Status.hpp"
#include "lib/type/Optional.hpp"

class HttpResponse {
 public:
  HttpResponse();
  explicit HttpResponse(lib::http::Status);
  ~HttpResponse();
  HttpResponse(const HttpResponse& other);
  HttpResponse& operator=(const HttpResponse& other);

  void SetStatus(lib::http::Status status);
  void SetStatus(lib::http::Status status, const std::string& reason_phrase);
  lib::http::Status GetStatus();
  void AddHeader(const std::string& key, const std::string& value);
  lib::type::Optional<std::string> GetHeader(const std::string& key);
  bool HasHeader(const std::string& key);
  void SetBody(const std::string& body);
  std::string GetBody() const;
  std::string ToHttpString() const;

  void EnsureDefaultErrorContent();  // sugar
  static std::string MakeDefaultErrorPage(int status_code,
                                          const std::string& reason_phrase);

 private:
  int status_code_;
  std::string reason_phrase_;
  std::map<std::string, std::string> headers_;
  std::string body_;
  std::string version_;
  void SetCurrentDateHeader();
};

#endif  // HTTPRESPONSE_HPP_
