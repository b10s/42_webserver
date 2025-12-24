#ifndef REQUESTHANDLER_HPP_
#define REQUESTHANDLER_HPP_

#include "HttpRequest.hpp"
#include "HttpResponse.hpp"
#include "ServerConfig.hpp"

class RequestHandler {
 public:
  RequestHandler(ServerConfig conf, HttpRequest req);
  ~RequestHandler();

  HttpResponse Run();

 private:
  RequestHandler();  // shouldn't use default constructor
  ServerConfig conf_;
  HttpRequest req_;
  HttpResponse res_;

  std::string full_path_;
  void HandleGet();
  void HandlePost();
  void HandleDelete();

  std::string ReadFile(std::string& filename);
};

#endif
