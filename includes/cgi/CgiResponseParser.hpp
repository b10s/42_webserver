#ifndef CGI_RESPONSE_PARSER_HPP_
#define CGI_RESPONSE_PARSER_HPP_

#include <string>

#include "HttpResponse.hpp"

namespace cgi {

HttpResponse ParseCgiResponse(const std::string& cgi_output);

}  // namespace cgi

#endif  // CGI_RESPONSE_PARSER_HPP_
