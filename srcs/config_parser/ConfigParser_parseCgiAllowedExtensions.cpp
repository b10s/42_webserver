#include "ConfigParser.hpp"

void ConfigParser::ParseCgiAllowedExtensions(Location* location) {
  if (location->HasCgiAllowedExtensions()) {
    throw std::runtime_error("Duplicate cgi allowed extensions directive");
  }
  std::string token = Tokenize(content);
  if (token.empty() || token == ";") {
    throw std::runtime_error(
        "Syntax error: expected cgi allowed extensions but got empty token or "
        ";");
  }

  while (!token.empty() && token != ";") {
    location->AddCgiAllowedExtension(token);
    token = Tokenize(content);
  }
  location->SetHasCgiAllowedExtensions(true);
}
