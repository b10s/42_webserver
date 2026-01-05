#ifndef CGI_RESPONSE_PARSER_HPP_
#define CGI_RESPONSE_PARSER_HPP_

#include <string>

#include "HttpResponse.hpp"
#include "lib/parser/StreamParser.hpp"

namespace cgi {

class CgiResponseParser : public lib::parser::StreamParser {
 public:
  CgiResponseParser();
  virtual ~CgiResponseParser();

  const HttpResponse& GetResponse() const;

 protected:
  virtual bool AdvanceHeader();
  virtual bool AdvanceBody();
  virtual void OnInternalStateError();
  virtual void OnExtraDataAfterDone();

 private:
  HttpResponse response_;
  void StoreHeader(const std::string& key, const std::string& value);
};

HttpResponse ParseCgiResponse(const std::string& cgi_output);

}  // namespace cgi

#endif  // CGI_RESPONSE_PARSER_HPP_
