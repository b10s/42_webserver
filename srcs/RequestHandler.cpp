#include "RequestHandler.hpp"

#include <iostream>

#include "HttpRequest.hpp"
#include "ServerConfig.hpp"
#include "lib/http/Method.hpp"
#include "lib/http/MimeType.hpp"
#include "lib/http/Status.hpp"
#include "lib/utils/ReadFile.hpp"

RequestHandler::RequestHandler() {
}

RequestHandler::RequestHandler(ServerConfig conf, HttpRequest req)
    : conf_(conf), req_(req) {
  // very simple file path sample
  full_path_ = conf.GetLocations()[0].GetRoot() + req.GetUri() +
               conf.GetLocations()[0].GetIndexFiles()[0];
}

RequestHandler::~RequestHandler() {
}

HttpResponse RequestHandler::Run() {
  lib::http::Method method = req_.GetMethod();
  if (method == lib::http::kGet) {
    HandleGet();
  } else if (method == lib::http::kPost) {
  } else if (method == lib::http::kDelete) {
  } else {
  }
  return res_;
}

// if uri ends with '/', it's a directory so append index file
// otherwise return as is
// TODO: check file existence and permissions, detect dangerous paths (e.g.,
// ../)
std::string RequestHandler::ResolveFullPath() const {
  std::string path = conf_.GetLocations()[0].GetRoot() + req_.GetUri();
  if (!path.empty() &&
      path[path.size() - 1] == '/') {  // if path ends with '/', it's a
                                       // directory so append index file
    path += conf_.GetLocations()[0].GetIndexFiles()[0];
  }
  return path;
}

void RequestHandler::HandleGet() {
  const std::string path = ResolveFullPath();
  std::string body = lib::utils::ReadFile(path);
  res_.AddHeader("Content-Type", lib::http::DetectMimeTypeFromPath(path));
  res_.SetBody(body);
  res_.SetStatus(lib::http::kOk);
}

void RequestHandler::HandlePost() {
}

void RequestHandler::HandleDelete() {
}
