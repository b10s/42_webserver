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
    const std::string index = location_match_.loc->GetIndexFile();
    if (index.empty()) {
      throw std::runtime_error(
          "No index files configured for location");  // TODO: status 500?
    }
    if (path.empty() || path[path.size() - 1] != '/') path += '/';
    path += index;
  }
  path = FileValidator::ValidateAndNormalizePath(
      path, location_match_.loc->GetRoot());
  return path;
}

void RequestHandler::HandleGet() {
  std::string path_with_index =
      AppendIndexFileIfDirectoryOrThrow(filesystem_path_);
  if (location_match_.loc->GetCgiEnabled()) {
    CgiExecutor cgi(req_, *location_match_.loc, path_with_index);
    result_ = cgi.Run();
  } else {
    std::string body = lib::utils::ReadFileToStringOrThrow(path_with_index);
    HttpResponse res;
    res.AddHeader("Content-Type",
                  lib::http::DetectMimeTypeFromPath(path_with_index));
    res.SetBody(body);
    res.SetStatus(lib::http::kOk);
    result_ = ExecResult(res);
  }
}

// reject directories for POST requests
void RequestHandler::HandlePost() {
  const std::string req_uri = req_.GetUri();
  if (!req_uri.empty() && req_uri[req_uri.size() - 1] == '/') {
    throw lib::exception::ResponseStatusException(lib::http::kBadRequest);
  }
  const std::string path = filesystem_path_;
  if (lib::utils::IsDirectory(path)) {
    throw lib::exception::ResponseStatusException(lib::http::kBadRequest);
  }
  if (location_match_.loc->GetCgiEnabled()) {
    CgiExecutor cgi(req_, *location_match_.loc, path);
    result_ = cgi.Run();
  } else {
    std::ofstream ofs(path.c_str(), std::ios::binary);
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
    res.AddHeader("Location", req_uri);  // TODO: should this be absolute URI?
    result_ = ExecResult(res);
  }
}

/*
RFC9110 Section 9.3.5 DELETE
If a DELETE method is successfully applied, the origin server SHOULD send
a 202 (Accepted) status code if the action will likely succeed but has not yet
been enacted, a 204 (No Content) status code if the action has been enacted and
no further information is to be supplied, or a 200 (OK) status code if the
action has been enacted and the response message includes a representation
describing the status.
*/
void RequestHandler::HandleDelete() {
  lib::utils::StatOrThrow(filesystem_path_);  // 404 if not found
  if (lib::utils::IsDirectory(filesystem_path_)) {
    throw lib::exception::ResponseStatusException(
        lib::http::kForbidden);  // Or BadRequest?
  }
  lib::utils::CheckDeletableRegularFileOrThrow(filesystem_path_); // 403 if no
                                                                   // permission
  if (std::remove(filesystem_path_.c_str()) != 0) {
    throw lib::exception::ResponseStatusException(lib::http::kForbidden);
  }
  HttpResponse res(lib::http::kOk);  // 200 OK
  res.SetBody("File deleted successfully");
  res.AddHeader("Content-Type", "text/plain");
  result_ = ExecResult(res);
}
