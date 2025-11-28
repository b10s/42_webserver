#include "HttpResponse.hpp"

#include <sstream>

HttpResponse::HttpResponse()
    : status_code_(200), reason_phrase_("OK"), version_("HTTP/1.1") {
}

HttpResponse::~HttpResponse() {
}

void HttpResponse::SetStatus(int status, const std::string& reason_phrase) {
  status_code_ = status;
  reason_phrase_ = reason_phrase;
}

void HttpResponse::AddHeader(const std::string& key, const std::string& value) {
  headers_[key] = value;
}

void HttpResponse::SetBody(const std::string& body) {
  body_ = body;
}

std::string HttpResponse::ToString() const {
  std::stringstream ss;

  // Status Line
  ss << version_ << " " << status_code_ << " " << reason_phrase_ << "\r\n";

  // Headers
  for (std::map<std::string, std::string>::const_iterator it = headers_.begin();
       it != headers_.end(); ++it) {
    ss << it->first << ": " << it->second << "\r\n";
  }

  // Content-Length (Auto-calculation)
  if (headers_.find("Content-Length") == headers_.end()) {
    ss << "Content-Length: " << body_.length() << "\r\n";
  }

  // End of Headers
  ss << "\r\n";

  // Body
  ss << body_;

  return ss.str();
}
