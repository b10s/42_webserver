#include "RequestHandler.hpp"

#include <stdexcept>

#include "CgiExecutor.hpp"
#include "FileValidator.hpp"
#include "HttpRequest.hpp"
#include "ServerConfig.hpp"
#include "lib/http/Method.hpp"
#include "lib/http/MimeType.hpp"
#include "lib/http/Status.hpp"
#include "lib/utils/file_utils.hpp"

RequestHandler::RequestHandler(ServerConfig conf, HttpRequest req)
    : conf_(conf), req_(req) {
}

RequestHandler::~RequestHandler() {
}

ExecResult RequestHandler::Run() {
  try {
    PrepareRoutingContext();

    if (location_match_.loc->HasRedirect()) {
      HttpResponse res;
      res.SetStatus(location_match_.loc->GetRedirectStatus());
      res.AddHeader("Location", location_match_.loc->GetRedirect());
      return ExecResult(res);
    }

    lib::http::Method method = req_.GetMethod();
    if (location_match_.loc->HasAllowedMethods()) {
      if (!location_match_.loc->IsMethodAllowed(method)) {
        return ExecResult(HttpResponse(lib::http::kMethodNotAllowed));
      }
    }

    if (method == lib::http::kGet) {
      HandleGet();
    } else if (method == lib::http::kPost) {
      HandlePost();
    } else if (method == lib::http::kDelete) {
      HandleDelete();
    } else {
      return ExecResult(HttpResponse(lib::http::kNotImplemented));
    }
    return result_;
  } catch (const lib::exception::ResponseStatusException& e) {
    HttpResponse res(e.GetStatus());
    res.SetBody(lib::http::StatusToString(e.GetStatus()));
    return ExecResult(res);
  } catch (const std::exception& e) {
    HttpResponse res(lib::http::kInternalServerError);
    res.SetBody("Internal Server Error");
    return ExecResult(res);
  }
}

void RequestHandler::PrepareRoutingContext() {
  const std::string req_uri = req_.GetUri();
  location_match_ = conf_.FindLocationForUri(req_uri);
  filesystem_path_ = ResolveFilesystemPath();
}

/*
Resolves the request URI into an absolute filesystem path
suitable for open(), stat(), and read().

Precondition:
- location root is an absolute path
- remainder starts with '/'
*/

// TODO:  POSTとDELETEで分ける
std::string RequestHandler::ResolveFilesystemPath() const {
  if (location_match_.loc ==
      NULL) {  // Defensive check for direct calls to
               // ResolveFilesystemPath() without a prior
               // PrepareRoutingContext(); in normal flows
               // FindLocationForUri() throws on not found.
    throw lib::exception::ResponseStatusException(lib::http::kNotFound);
  }
  if (location_match_.loc->HasRedirect()) {
    return "";
  }
  std::string path = location_match_.loc->GetRoot() + location_match_.remainder;
  // Validate/normalize (security)
  path = FileValidator::ValidateAndNormalizePath(
      path, location_match_.loc->GetRoot());
  return path;
}

std::string RequestHandler::AppendIndexFileIfDirectoryOrThrow(
    const std::string& base_path) const {
  const std::string req_uri = req_.GetUri();
  const bool req_uri_ends_with_slash =
      (!req_uri.empty() && req_uri[req_uri.size() - 1] == '/');
  bool is_directory =
      (req_uri_ends_with_slash || lib::utils::IsDirectory(base_path));
  /* TODO: replace lib::utils::IsDirectory() with StatOrThrow + S_ISDIR later
  bool is_directory = false;
  if (req_uri_ends_with_slash) {
    is_directory = true;
  } else {
    struct stat st = lib::utils::StatOrThrow(path);
    is_directory = S_ISDIR(st.st_mode);
  }
  */
  std::string path = base_path;
  if (is_directory) {
    if (location_match_.loc->GetIndexFile().empty()) {
      throw std::runtime_error(
          "No index files configured for location");  // TODO: status 500?
    }
    if (path.empty() || path[path.size() - 1] != '/') path += '/';
    path += location_match_.loc->GetIndexFile();
  }
  path = FileValidator::ValidateAndNormalizePath(
      path, location_match_.loc->GetRoot());
  return path;
}

void RequestHandler::HandleGet() {
  if (location_match_.loc->GetCgiEnabled()) {
    CgiExecutor cgi(req_, *location_match_.loc, filesystem_path_);
    result_ = cgi.Run();
  } else {
    std::string body = lib::utils::ReadFileToStringOrThrow(filesystem_path_);
    HttpResponse res;
    res.AddHeader("Content-Type",
                  lib::http::DetectMimeTypeFromPath(filesystem_path_));
    res.SetBody(body);
    res.SetStatus(lib::http::kOk);
    result_ = ExecResult(res);
  }
}

void RequestHandler::HandlePost() {
  if (location_match_.loc->GetCgiEnabled()) {
    CgiExecutor cgi(req_, *location_match_.loc, filesystem_path_);
    result_ = cgi.Run();
  } else {
    std::ofstream ofs(filesystem_path_.c_str(), std::ios::binary);
    if (!ofs) {
      throw lib::exception::ResponseStatusException(lib::http::kForbidden);
      // response_->setStatus(kForbidden); // shoud we check errno and return
      // 403/404/500 accordingly? return;
    }
    const std::string& req_body = req_.GetBody();
    ofs.write(req_body.data(), static_cast<std::streamsize>(req_body.size()));
    if (!ofs) {
      throw lib::exception::ResponseStatusException(
          lib::http::kInternalServerError);
      // response_->setStatus(kInternalServerError);
      // return;
    }
    HttpResponse res(lib::http::kCreated);  // 201 Created
    res.AddHeader("Content-Length", "0");
    result_ = ExecResult(res);
  }
}

void RequestHandler::HandleDelete() {
  if (location_match_.loc->GetCgiEnabled()) {
    CgiExecutor cgi(req_, *location_match_.loc, filesystem_path_);
    result_ = cgi.Run();
  } else {
    std::string body = lib::utils::ReadFileToStringOrThrow(filesystem_path_);
    HttpResponse res;
    res.AddHeader("Content-Type",
                  lib::http::DetectMimeTypeFromPath(filesystem_path_));
    res.SetBody(body);
    res.SetStatus(lib::http::kOk);
    result_ = ExecResult(res);
  }
}
