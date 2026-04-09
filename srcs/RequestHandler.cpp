#include "RequestHandler.hpp"

#include <stdexcept>

#include "CgiExecutor.hpp"
#include "FileValidator.hpp"
#include "HttpRequest.hpp"
#include "ServerConfig.hpp"
#include "lib/exception/ResponseStatusException.hpp"
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
        HttpResponse res = BuildErrorResponse(lib::http::kMethodNotAllowed);
        res.AddHeader("Allow", location_match_.loc->GetAllowedMethodsString());
        return ExecResult(res);
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
    return ExecResult(BuildErrorResponse(e.GetStatus()));
  } catch (const std::exception& e) {
    return ExecResult(BuildErrorResponse(lib::http::kInternalServerError));
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
  std::cerr << "[DEBUG] location root: " << location_match_.loc->GetRoot()
            << ", request URI: " << req_.GetUri()
            << ", remainder: " << location_match_.remainder
            << ", combined path: " << path << std::endl;
  // Validate/normalize (security)
  path = FileValidator::ValidateAndNormalizePath(
      path, location_match_.loc->GetRoot());
  return path;
}

void RequestHandler::HandleGet() {
  const std::string req_uri = req_.GetUri();
  if (lib::utils::IsDirectory(filesystem_path_)) {
    if (req_uri.empty() || req_uri[req_uri.size() - 1] != '/') {
      HttpResponse res;
      res.SetStatus(lib::http::kMovedPermanently);
      std::string location = req_uri + "/";
      const std::string query = req_.GetQuery();
      if (!query.empty()) {
        location += "?" + query;
      }
      res.AddHeader("Location", location);
      result_ = ExecResult(res);
      return;
    }
  }
  GetTarget target = ResolveGetTarget();
  if (target.type == GetTarget::kAutoIndex) {
    HttpResponse res;
    res.SetStatus(lib::http::kOk);
    res.AddHeader("Content-Type", "text/html");
    res.SetBody(target.body);
    result_ = ExecResult(res);
    return;
  }
  if (target.type == GetTarget::kCgi) {
    CgiExecutor cgi(req_, *location_match_.loc, target.path);
    result_ = cgi.Run();
    return;
  }
  std::string body = lib::utils::ReadFileToStringOrThrow(target.path);
  HttpResponse res;
  res.AddHeader("Content-Type", lib::http::DetectMimeTypeFromPath(target.path));
  res.SetBody(body);
  res.SetStatus(lib::http::kOk);
  result_ = ExecResult(res);
}

// reject directories for POST requests
// nginx returns the 405 status code for POST method
// requesting a static file only if the file exists.
void RequestHandler::HandlePost() {
  const std::string req_uri = req_.GetUri();
  if (!req_uri.empty() && req_uri[req_uri.size() - 1] == '/') {
    std::cerr
        << "[DEBUG] POST request URI ends with '/', rejecting as directory"
        << std::endl;
    HttpResponse res;
    res.SetStatus(lib::http::kMethodNotAllowed);
    res.AddHeader("Connection", "close");
    res.AddHeader("Content-Type", "text/html");
    res.EnsureDefaultErrorContent();
    result_ = ExecResult(res);
    return;
  }
  const std::string path = filesystem_path_;
  if (lib::utils::IsDirectory(path)) {
    std::cerr << "[DEBUG] POST request resolved to a directory, rejecting"
              << std::endl;
    HttpResponse res;
    res.SetStatus(lib::http::kMethodNotAllowed);  // 405
    res.AddHeader("Connection", "close");
    res.AddHeader("Content-Type", "text/html");
    res.EnsureDefaultErrorContent();
    result_ = ExecResult(res);
    return;
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
  lib::utils::CheckDeletableRegularFileOrThrow(filesystem_path_);  // 403 if no
                                                                   // permission
  if (std::remove(filesystem_path_.c_str()) != 0) {
    throw lib::exception::ResponseStatusException(
        lib::utils::MapErrnoToHttpStatus(errno));
  }
  HttpResponse res(lib::http::kOk);  // 200 OK
  res.SetBody("File deleted successfully");
  res.AddHeader("Content-Type", "text/plain");
  result_ = ExecResult(res);
}

GetTarget RequestHandler::ResolveGetTarget() {
  GetTarget target;
  std::string path = filesystem_path_;

  const std::string req_uri = req_.GetUri();
  // if path is a directory, try to append index file or generate autoindex
  if (lib::utils::IsDirectory(path)) {
    const std::string index = location_match_.loc->GetIndexFile();
    if (!index.empty()) {  // if index file specified, try to serve it
      std::string index_path = path;
      if (index_path[index_path.size() - 1] != '/') index_path += "/";
      index_path += index;

      struct stat st;
      if (::stat(index_path.c_str(), &st) == 0 && S_ISREG(st.st_mode)) {
        path = index_path;
      } else {
        if (location_match_.loc
                ->GetAutoIndex()) {  // if index file can't be served but
                                     // autoindex enabled, generate autoindex
          target.type = GetTarget::kAutoIndex;
          target.body = GenerateDirectoryListing(path, req_uri);
          return target;
        }
        throw lib::exception::ResponseStatusException(lib::http::kForbidden);
      }
    } else {  // if no index file specified, try to serve directory listing if
              // autoindex enabled
      if (location_match_.loc->GetAutoIndex()) {
        target.type = GetTarget::kAutoIndex;
        target.body = GenerateDirectoryListing(path, req_uri);
        return target;
      }
      throw lib::exception::ResponseStatusException(lib::http::kForbidden);
    }
  }
  // CGI check should be after directory/index resolution
  // as CGI can be enabled for a directory but the actual request target is a
  // file.
  if (location_match_.loc->GetCgiEnabled()) {
    target.type = GetTarget::kCgi;
    target.path = path;
    return target;
  }
  target.type = GetTarget::kStaticFile;
  target.path = path;
  return target;
}

FileEntry::FileEntry(const std::string& file_name,
                     const std::string& last_modified_time,
                     lib::type::Optional<long> file_size, bool is_directory)
    : file_name(file_name),
      last_modified_time(last_modified_time),
      file_size(file_size),
      is_directory(is_directory) {
}

std::string RequestHandler::GenerateDirectoryListing(
    const std::string& path, const std::string& req_uri) const {
  DIR* dir;
  struct dirent* entry;
  struct stat file_stat;
  std::stringstream html;
  std::vector<FileEntry> entries;

  dir = opendir(path.c_str());
  if (dir == NULL) {
    throw lib::exception::ResponseStatusException(lib::http::kForbidden);
  }
  while ((entry = readdir(dir)) != NULL) {
    std::string file_name(entry->d_name);
    std::string full_path = path + "/" + file_name;
    if (file_name == "." || file_name == "..") {
      continue;  // skip current and parent directory entries
    }
    if (stat(full_path.c_str(), &file_stat) == -1) {
      continue;  // skip files that can't be stat-ed
    }
    bool is_directory = S_ISDIR(file_stat.st_mode);
    char time_buf[20];
    struct tm tm;
    localtime_r(&file_stat.st_mtime, &tm);
    strftime(time_buf, sizeof(time_buf), "%d-%b-%Y %H:%M", &tm);
    entries.push_back(
        FileEntry(file_name, std::string(time_buf),
                  is_directory ? lib::type::Optional<long>()
                               : lib::type::Optional<long>(file_stat.st_size),
                  is_directory));
  }
  closedir(dir);
  // sort by file name
  for (size_t i = 0; i < entries.size(); ++i) {
    for (size_t j = i + 1; j < entries.size(); ++j) {
      if (entries[i].file_name > entries[j].file_name) {
        std::swap(entries[i], entries[j]);
      }
    }
  }

  // Generate HTML
  html
      << "<!DOCTYPE html>\n"
      << "<html lang=\"en\">\n"
      << "<head>\n"
      << "    <meta charset=\"UTF-8\">\n"
      << "    <meta name=\"viewport\" content=\"width=device-width, "
         "initial-scale=1.0\">\n"
      << "    <title>Index of " << req_uri << "</title>\n"
      << "    <style>\n"
      << "        body { font-family: Arial, sans-serif; margin: 20px; }\n"
      << "        h1 { border-bottom: 1px solid #ccc; padding-bottom: 10px; }\n"
      << "        table { border-collapse: collapse; width: 100%; }\n"
      << "        th, td { text-align: left; padding: 8px; }\n"
      << "        tr:nth-child(even) { background-color: #f2f2f2; }\n"
      << "    </style>\n"
      << "</head>\n"
      << "<body>\n"
      << "    <h1>Index of " << req_uri << "</h1>\n"
      << "    <table>\n"
      << "        <tr>\n"
      << "            <th>Name</th>\n"
      << "            <th>Last modified</th>\n"
      << "            <th>Size</th>\n"
      << "        </tr>\n";

  html << "        <tr>\n"
       << "            <td><a href=\"../\">../</a></td>\n"
       << "            <td></td>\n"
       << "            <td></td>\n"
       << "        </tr>\n";

  for (std::vector<FileEntry>::const_iterator it = entries.begin();
       it != entries.end(); ++it) {
    html << "        <tr>\n"
         << "            <td><a href=\"" << it->file_name
         << (it->is_directory ? "/" : "") << "\">" << it->file_name
         << (it->is_directory ? "/" : "") << "</a></td>\n"
         << "            <td>" << it->last_modified_time << "</td>\n"
         << "            <td>"
         << (it->is_directory
                 ? "-"
                 : lib::utils::OptionalLongToString(it->file_size).ValueOr(""))
         << "</td>\n"
         << "        </tr>\n";
  }

  html << "    </table>\n"
       << "</body>\n"
       << "</html>";

  return html.str();
}

HttpResponse RequestHandler::BuildErrorResponse(lib::http::Status status) {
  HttpResponse res(status);
  res.AddHeader("Content-Type", "text/html");
  res.AddHeader("Connection", "close");

  try {
    if (conf_.HasErrorPage(status)) {
      const std::string& custom_error_path = conf_.GetErrorPage(status);
      std::cerr << "[DEBUG] loading custom error page for status " << status
                << " from path: " << custom_error_path << std::endl;
      try {  // try direct path first
        std::string body =
            lib::utils::ReadFileToStringOrThrow(custom_error_path);
        res.SetBody(body);
        return res;
      } catch (const std::exception& e) {
        std::cerr << "[DEBUG] failed to read error page at '"
                  << custom_error_path << "': " << e.what() << std::endl;
      }
      if (!location_match_.loc || custom_error_path.empty() ||
          custom_error_path[0] != '/') {
        // nothing more to try
      } else {
        std::string alt = location_match_.loc->GetRoot();
        if (!alt.empty() && alt[alt.size() - 1] == '/')
          alt.resize(alt.size() - 1);
        std::string candidate =
            alt + custom_error_path;  // location_root + "/errors/404.html"
        std::cerr << "[DEBUG] trying alternative error page path: '"
                  << candidate << "'" << std::endl;
        try {
          std::string body = lib::utils::ReadFileToStringOrThrow(candidate);
          res.SetBody(body);
          return res;
        } catch (const std::exception& e) {
          std::cerr << "[DEBUG] failed to read alternative error page at '"
                    << candidate << "': " << e.what() << std::endl;
        }
      }
    }
  } catch (...) {
    // fall back to default error page
    // if any error occurs while reading custom error page
  }
  res.EnsureDefaultErrorContent();
  return res;
}
