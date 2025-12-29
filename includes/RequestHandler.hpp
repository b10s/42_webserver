#ifndef REQUESTHANDLER_HPP_
#define REQUESTHANDLER_HPP_

#include "HttpRequest.hpp"
#include "HttpResponse.hpp"
#include "LocationMatch.hpp"
#include "ServerConfig.hpp"

class RequestHandler {
 public:
  RequestHandler(ServerConfig conf, HttpRequest req);
  ~RequestHandler();

  HttpResponse Run();
  void PrepareRoutingContext();
  std::string ResolveFilesystemPath() const;  // for testing purpose

  // for tests (read-only)
  const LocationMatch& GetLocationMatchForTest() const {
    return location_match_;
  }

  const std::string& GetFilesystemPathForTest() const {
    return filesystem_path_;
  }

 private:
  RequestHandler();  // shouldn't use default constructor
  ServerConfig conf_;
  HttpRequest req_;
  HttpResponse res_;

  LocationMatch location_match_;
  std::string filesystem_path_;
  void HandleGet();
  void HandlePost();
  void HandleDelete();
};

#endif
