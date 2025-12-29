#include "cgi/CgiResponseParser.hpp"

#include <iostream>
#include <sstream>
#include <string>

#include "HttpResponse.hpp"
#include "lib/http/Status.hpp"
#include "lib/type/Optional.hpp"
#include "lib/utils/string_utils.hpp"

namespace cgi {

HttpResponse ParseCgiResponse(const std::string& cgi_output) {
  HttpResponse response(lib::http::kOk);

  // Find separation between headers and body
  size_t header_end = cgi_output.find("\r\n\r\n");
  if (header_end == std::string::npos) {
    header_end = cgi_output.find("\n\n");
  }

  if (header_end == std::string::npos) {
    // RFC 3875 Violation: No header-body separator
    response.SetStatus(lib::http::kInternalServerError);
    response.EnsureDefaultErrorContent();
    return response;
  }

  std::string headers_part = cgi_output.substr(0, header_end);
  response.SetBody(cgi_output.substr(
      header_end + ((cgi_output[header_end] == '\r') ? 4 : 2)));

  std::stringstream ss(headers_part);
  std::string line;
  while (std::getline(ss, line)) {
    if (!line.empty() && line[line.length() - 1] == '\r')
      line.erase(line.length() - 1);
    if (line.empty()) continue;

    size_t colon = line.find(':');
    if (colon != std::string::npos) {
      std::string key = line.substr(0, colon);
      std::string val = line.substr(colon + 1);

      size_t val_start = 0;
      while (val_start < val.length() && std::isspace(val[val_start]))
        val_start++;
      val = val.substr(val_start);

      if (key == "Status") {
        size_t space_pos = val.find(' ');
        std::string status_str;
        std::string reason_phrase;
        if (space_pos != std::string::npos) {
          status_str = val.substr(0, space_pos);
          reason_phrase = val.substr(space_pos + 1);
        } else {
          status_str = val;
          reason_phrase = "";
        }
        lib::type::Optional<long> status_code_opt =
            lib::utils::StrToLong(status_str);
        if (status_code_opt.HasValue()) {
          response.SetStatus(
              static_cast<lib::http::Status>(status_code_opt.Value()));
        } else {
          response.SetStatus(lib::http::kInternalServerError);
          response.EnsureDefaultErrorContent();
          return response;
        }
      } else if (key == "Location") {
        if (response.GetStatus() == lib::http::kOk) {
          response.SetStatus(lib::http::kFound);
        }
        response.AddHeader(key, val);
      } else {
        response.AddHeader(key, val);
      }
    } else {
      response.SetStatus(lib::http::kInternalServerError);
      response.EnsureDefaultErrorContent();
      return response;
    }
  }

  if (!response.HasHeader("content-type")) {
    response.SetStatus(lib::http::kInternalServerError);
    response.EnsureDefaultErrorContent();
    return response;
  }
  return response;
}

}  // namespace cgi
