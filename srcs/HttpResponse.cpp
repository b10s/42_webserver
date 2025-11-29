#include "HttpResponse.hpp"

#include <ctime>
#include <sstream>

#include "lib/exception/InvalidHeader.hpp"  // Include the new exception
#include "lib/utils/string_utils.hpp"

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
  if (key.find('\r') != std::string::npos ||
      key.find('\n') != std::string::npos ||
      value.find('\r') != std::string::npos ||
      value.find('\n') != std::string::npos) {
    throw lib::exception::InvalidHeader();
  }
  headers_[lib::utils::ToLowerAscii(key)] = value;
}

void HttpResponse::SetBody(const std::string& body) {
  body_ = body;
}

std::string HttpResponse::ToString() const {
  std::stringstream ss;

  // Status Line
  ss << version_ << " " << status_code_ << " " << reason_phrase_ << "\r\n";

  // Headers
  std::map<std::string, std::string> final_headers = headers_;

  // Date Header
  bool has_date = final_headers.count("date");
  if (!has_date) {
    std::time_t now = std::time(NULL);
    std::tm* tm = std::gmtime(&now);
    char buf[100];
    if (std::strftime(buf, sizeof(buf), "%a, %d %b %Y %H:%M:%S GMT", tm)) {
      final_headers["date"] = std::string(buf);
    }
  }

  // Content-Length
  bool has_content_length = final_headers.count(
      "content-length");  // Key for content-length may be present from user
  // RFC 7230 Section 3.3.2: A sender MUST NOT send a Content-Length header
  // field in any message that contains a Transfer-Encoding header field.
  bool has_transfer_encoding = final_headers.count("transfer-encoding");
  if (!has_content_length && !has_transfer_encoding) {
    std::stringstream len_ss;
    len_ss << body_.length();
    final_headers["content-length"] = len_ss.str();
  }

  // Output Headers
  for (std::map<std::string, std::string>::const_iterator it =
           final_headers.begin();
       it != final_headers.end(); ++it) {
    ss << it->first << ": " << it->second << "\r\n";
  }

  // End of Headers
  ss << "\r\n";

  // Body
  ss << body_;

  return ss.str();
}
