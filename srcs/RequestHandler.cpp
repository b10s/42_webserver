#include "RequestHandler.hpp"

#include <iostream>
#include <stdexcept>

#include "CgiExecutor.hpp"
#include "HttpRequest.hpp"
#include "ServerConfig.hpp"
#include "lib/http/Method.hpp"
#include "lib/http/MimeType.hpp"
#include "lib/http/Status.hpp"
#include "lib/utils/file_utils.hpp"

RequestHandler::RequestHandler(ServerConfig conf, HttpRequest req)
    : conf_(conf), req_(req) {
  // location_match_.loc = NULL;
  PrepareRoutingContext();
}

RequestHandler::~RequestHandler() {
}

HttpResponse RequestHandler::Run() {
  // PrepareRoutingContext();
  lib::http::Method method = req_.GetMethod();
  if (method == lib::http::kGet) {
    HandleGet();
  } else if (method == lib::http::kPost) {
    HandlePost();
  } else if (method == lib::http::kDelete) {
    HandleDelete();
  } else {
    return HttpResponse(lib::http::kNotImplemented);
  }
  return res_;
}

void RequestHandler::PrepareRoutingContext() {
  const std::string req_uri = req_.GetUri();
  location_match_ = conf_.FindLocationForUri(req_uri);
  filesystem_path_ = ResolveFilesystemPath();
}

/*
Resolves the request URI into an absolute filesystem path
suitable for open(), stat(), and read().

TODO: check file existence and permissions, detect dangerous paths
return 308 if uri is a directory but missing trailing '/' (normalize)?
*/
std::string RequestHandler::ResolveFilesystemPath() const {
  if (location_match_.loc == NULL) {
    throw std::runtime_error("No matching location found for URI: " +
                             req_.GetUri());  // TODO: return HTTP 404?
  }
  const std::string req_uri = req_.GetUri();
  std::string path = location_match_.loc->GetRoot() + location_match_.remainder;
  bool req_uri_ends_with_slash =
      (!req_uri.empty() && req_uri[req_uri.size() - 1] == '/');
  bool is_directory =
      (req_uri_ends_with_slash || lib::utils::IsDirectory(path));
  if (is_directory) {
    if (location_match_.loc->GetIndexFile().empty()) {
      throw std::runtime_error("No index files configured for location");
    }
    if (path.empty() || path[path.size() - 1] != '/') path += '/';
    path += location_match_.loc->GetIndexFile();
  }
  return path;
}

void RequestHandler::HandleGet() {
  if (location_match_.loc->GetCgiEnabled()) {
    CgiExecutor cgi(req_, *location_match_.loc, filesystem_path_);
    res_ = cgi.Run();
  } else {
    std::string body = lib::utils::ReadFile(filesystem_path_);
    res_.AddHeader("Content-Type",
                   lib::http::DetectMimeTypeFromPath(filesystem_path_));
    res_.SetBody(body);
    res_.SetStatus(lib::http::kOk);
  }
}

void RequestHandler::HandlePost() {
  if (location_match_.loc->GetCgiEnabled()) {
    CgiExecutor cgi(req_, *location_match_.loc, filesystem_path_);
    res_ = cgi.Run();
  } else {
    std::string body = lib::utils::ReadFile(filesystem_path_);
    res_.AddHeader("Content-Type",
                   lib::http::DetectMimeTypeFromPath(filesystem_path_));
    res_.SetBody(body);
    res_.SetStatus(lib::http::kOk);
  }
}

void RequestHandler::HandleDelete() {
  if (location_match_.loc->GetCgiEnabled()) {
    CgiExecutor cgi(req_, *location_match_.loc, filesystem_path_);
    res_ = cgi.Run();
  } else {
    std::string body = lib::utils::ReadFile(filesystem_path_);
    res_.AddHeader("Content-Type",
                   lib::http::DetectMimeTypeFromPath(filesystem_path_));
    res_.SetBody(body);
    res_.SetStatus(lib::http::kOk);
  }
}
