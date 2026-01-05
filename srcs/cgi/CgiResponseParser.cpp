#include "cgi/CgiResponseParser.hpp"

#include <iostream>
#include <sstream>
#include <string>

#include "HttpResponse.hpp"
#include "lib/exception/ResponseStatusException.hpp"
#include "lib/http/Status.hpp"
#include "lib/type/Optional.hpp"
#include "lib/utils/string_utils.hpp"

namespace cgi {

CgiResponseParser::CgiResponseParser() : response_(lib::http::kOk) {
}

CgiResponseParser::~CgiResponseParser() {
}

const HttpResponse& CgiResponseParser::GetResponse() const {
  return response_;
}

bool CgiResponseParser::AdvanceHeader() {
  std::string::size_type end_of_header = FindEndOfHeader(buffer_);
  if (end_of_header == std::string::npos) {
    return false;
  }

  const char* req = buffer_.c_str();
  size_t total_len = 0;
  // CGI headers can start immediately
  while (*req) {
    if (IsCRLF(req)) break;
    if (!IsStrictCrlf() && IsLF(req)) break;

    std::string key, value;
    req = ReadHeaderLine(req, key, value, total_len, 8192);
    StoreHeader(key, value);
  }

  buffer_.erase(0, end_of_header);
  state_ = kBody;
  return true;
}

void CgiResponseParser::StoreHeader(const std::string& key,
                                    const std::string& value) {
  if (key == "Status") {
    size_t space_pos = value.find(' ');
    std::string status_str;
    if (space_pos != std::string::npos) {
      status_str = value.substr(0, space_pos);
    } else {
      status_str = value;
    }
    lib::type::Optional<long> status_code_opt =
        lib::utils::StrToLong(status_str);
    if (status_code_opt.HasValue()) {
      response_.SetStatus(
          static_cast<lib::http::Status>(status_code_opt.Value()));
    } else {
      throw lib::exception::ResponseStatusException(
          lib::http::kInternalServerError);
    }
  } else if (key == "Location") {
    if (response_.GetStatus() == lib::http::kOk) {
      response_.SetStatus(lib::http::kFound);
    }
    response_.AddHeader(key, value);
  } else {
    response_.AddHeader(key, value);
  }
}

bool CgiResponseParser::AdvanceBody() {
  response_.SetBody(buffer_);
  buffer_.clear();
  state_ = kDone;
  return true;
}

void CgiResponseParser::OnInternalStateError() {
  response_.SetStatus(lib::http::kInternalServerError);
  response_.EnsureDefaultErrorContent();
  state_ = kDone;
}

void CgiResponseParser::OnExtraDataAfterDone() {
}

bool CgiResponseParser::IsStrictCrlf() const {
  return false;
}

HttpResponse ParseCgiResponse(const std::string& cgi_output) {
  CgiResponseParser parser;
  try {
    parser.Parse(cgi_output.c_str(), cgi_output.length());
  } catch (const std::exception&) {
    HttpResponse error_res(lib::http::kInternalServerError);
    error_res.EnsureDefaultErrorContent();
    return error_res;
  }

  HttpResponse res = parser.GetResponse();
  if (!res.HasHeader("content-type")) {
    res.SetStatus(lib::http::kInternalServerError);
    res.EnsureDefaultErrorContent();
  }
  return res;
}

}  // namespace cgi
