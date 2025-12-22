#ifndef HTTPRESPONSE_HPP_
#define HTTPRESPONSE_HPP_

#include <map>
#include <string>

#include "HttpRequest.hpp"
#include "lib/http/Method.hpp"

class HttpResponse {
 public:
  HttpResponse();
  ~HttpResponse();

  void SetStatus(int status, const std::string& reason_phrase);
  void AddHeader(const std::string& key, const std::string& value);
  void SetBody(const std::string& body);
  std::string GetBody() const;
  std::string ToHttpString() const;

  void EnsureDefaultBodyIfEmpty();  // sugar
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
