#include "RequestHandler.hpp"

#include <iostream>
#include <stdexcept>

#include "HttpRequest.hpp"
#include "ServerConfig.hpp"
#include "lib/http/Method.hpp"
#include "lib/http/MimeType.hpp"
#include "lib/http/Status.hpp"
#include "lib/utils/ReadFile.hpp"
#include "lib/utils/file_utils.hpp"

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
  const std::vector<Location>& locations = conf_.GetLocations();
  // Ensure we have at least one location
  if (locations.empty()) {
    throw std::runtime_error("No locations configured");
  }
  const Location& location = locations[0];
  if (location.GetRoot().empty()) {
    throw std::runtime_error("Location root is empty");
  }
  std::string path = location.GetRoot() + req_.GetUri();
  // Check if path is actually a directory (either ends with '/' or filesystem
  // says so)
  bool is_directory = (!path.empty() && path[path.size() - 1] == '/') ||
                      lib::utils::IsDirectory(path);
  if (is_directory) {
    if (location.GetIndexFiles().empty()) {
      throw std::runtime_error("No index files configured for location");
    }
    path += location.GetIndexFiles()[0];
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
