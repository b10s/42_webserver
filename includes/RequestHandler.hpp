#ifndef REQUESTHANDLER_HPP_
#define REQUESTHANDLER_HPP_

#include "ExecResult.hpp"
#include "HttpRequest.hpp"
#include "HttpResponse.hpp"
#include "LocationMatch.hpp"
#include "ServerConfig.hpp"

class RequestHandler {
 public:
  RequestHandler(ServerConfig conf, HttpRequest req);
  ~RequestHandler();

  ExecResult Run();
  void PrepareRoutingContext();
  std::string ResolveFilesystemPath() const;  // for testing purpose
  std::string AppendIndexFileIfDirectoryOrThrow(
      const std::string& base_path) const;

 private:
  RequestHandler();  // shouldn't use default constructor
  ServerConfig conf_;
  HttpRequest req_;
  ExecResult result_;

  LocationMatch location_match_;
  std::string filesystem_path_;
  void HandleGet();
  void HandlePost();
  void HandleDelete();
};

#endif
