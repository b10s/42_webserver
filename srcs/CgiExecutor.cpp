#include "CgiExecutor.hpp"

#include <sys/wait.h>
#include <unistd.h>

#include <cctype>
#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>

#include "HttpResponse.hpp"
#include "cgi/CgiResponseParser.hpp"
#include "lib/http/Status.hpp"
#include "lib/type/Optional.hpp"
#include "lib/utils/string_utils.hpp"

namespace {
std::vector<char*> CreateEnvp(const std::vector<std::string>& envs) {
  std::vector<char*> envp;
  for (size_t i = 0; i < envs.size(); ++i) {
    envp.push_back(const_cast<char*>(envs[i].c_str()));
  }
  envp.push_back(NULL);
  return envp;
}

void ClosePipe(int fd[2]) {
  close(fd[0]);
  close(fd[1]);
}

lib::type::Optional<std::string> CreatePathInfo(
    const std::string& path, const std::vector<std::string>& extensions) {
  size_t min_pos = std::string::npos;
  size_t found_ext_len = 0;

  for (size_t i = 0; i < extensions.size(); ++i) {
    const std::string& ext = extensions[i];
    if (ext.empty()) continue;

    // Search from the end of the path to find the most relevant occurrence
    // of the extension and ensure it is properly delimited within a
    // path component.
    size_t pos = path.rfind(ext);
    while (pos != std::string::npos) {
      size_t ext_end = pos + ext.length();

      // The extension must be at the end of the path or immediately
      // followed by a '/' to terminate the path component.
      if (ext_end == path.length() || path[ext_end] == '/') {
        // Additionally, ensure the extension is not directly preceded
        // by a '/' (which would make the extension a whole path
        // component like "/.py/"), unless it is at the very start
        // of the path.
        if (pos == 0 || path[pos - 1] != '/') {
          // Among all valid matches, keep the one that appears
          // earliest in the path to preserve existing semantics.
          if (pos < min_pos) {
            min_pos = pos;
            found_ext_len = ext.length();
          }
          break;
        }
      }

      if (pos == 0) {
        break;
      }
      pos = path.rfind(ext, pos - 1);
    }
  }

  if (min_pos != std::string::npos) {
    return lib::type::Optional<std::string>(
        path.substr(min_pos + found_ext_len));
  }
  return lib::type::Optional<std::string>();
}

bool IsScriptExtensionAllowed(const std::string& script_path,
                              const std::vector<std::string>& extensions) {
  for (size_t i = 0; i < extensions.size(); ++i) {
    const std::string& ext = extensions[i];
    if (script_path.length() >= ext.length() &&
        script_path.compare(script_path.length() - ext.length(), ext.length(),
                            ext) == 0) {
      return true;
    }
  }
  return false;
}

// void PrintEnvp(const std::vector<char*>& envp) {
//   for (size_t i = 0; envp[i] != NULL; ++i) {
//     std::cout << "envp[" << i << "]: " << envp[i] << std::endl;
//   }
// }
}  // namespace

CgiExecutor::CgiExecutor(const HttpRequest& req, const Location& loc,
                         const std::string& script_path)
    : loc_(loc), script_path_(script_path), body_(req.GetBody()) {
  InitializeMetaVars(req);
}

CgiExecutor::~CgiExecutor() {
}

std::string CgiExecutor::GetMetaVar(const std::string& key) const {
  return meta_vars_.at(key).Value();
}

std::vector<std::string> CgiExecutor::GetMetaVars() const {
  // store as vector of "key=value" string
  std::vector<std::string> ret_vars;

  for (std::map<std::string, lib::type::Optional<std::string> >::const_iterator
           it = meta_vars_.begin();
       it != meta_vars_.end(); ++it) {
    if (it->second.HasValue()) {
      ret_vars.push_back(it->first + "=" + it->second.Value());
    }
  }
  return ret_vars;
}

void CgiExecutor::InitializeMetaVars(const HttpRequest& req) {
  lib::type::Optional<std::string> path_info =
      CreatePathInfo(script_path_, loc_.GetCgiAllowedExtensions());
  std::string executable_path =
      path_info.HasValue()
          ? script_path_.substr(
                0, script_path_.length() - path_info.Value().length())
          : script_path_;
  // RFC 3875 4.1.1.
  lib::type::Optional<std::string> auth = req.GetHeader("authorization");
  meta_vars_["AUTH_TYPE"] = auth.HasValue()
                                ? lib::utils::GetFirstToken(auth.Value(), " ")
                                : lib::type::Optional<std::string>();
  // RFC 3875 4.1.2.
  meta_vars_["CONTENT_LENGTH"] = req.GetHeader("content-length");
  // RFC 3875 4.1.3.
  meta_vars_["CONTENT_TYPE"] = req.GetHeader("content-type");
  // RFC 3875 4.1.4.
  meta_vars_["GATEWAY_INTERFACE"] = lib::type::Optional<std::string>("CGI/1.1");
  // RFC 3875 4.1.5.
  meta_vars_["PATH_INFO"] = path_info;
  // RFC 3875 4.1.6.
  meta_vars_["PATH_TRANSLATED"] =
      path_info.HasValue()
          ? lib::type::Optional<std::string>(loc_.GetRoot() + path_info.Value())
          : lib::type::Optional<std::string>();
  // RFC 3875 4.1.7.
  meta_vars_["QUERY_STRING"] = lib::type::Optional<std::string>(req.GetQuery());
  // RFC 3875 4.1.8.
  meta_vars_["REMOTE_ADDR"] =
      lib::type::Optional<std::string>(req.GetClientIp());
  // RFC 3875 4.1.9.
  meta_vars_["REMOTE_HOST"] =
      lib::type::Optional<std::string>(req.GetClientIp());
  // RFC 3875 4.1.10.
  // The server may choose not to support this feature, or not to request the
  // data for efficiency reasons, or not to return available identity data.
  meta_vars_["REMOTE_IDENT"] = lib::type::Optional<std::string>();
  // RFC 3875 4.1.11.
  // The server doesn't support authentication yet.
  meta_vars_["REMOTE_USER"] = lib::type::Optional<std::string>();
  // RFC 3875 4.1.12.
  meta_vars_["REQUEST_METHOD"] = lib::type::Optional<std::string>(
      lib::http::MethodToString(req.GetMethod()));
  // RFC 3875 4.1.13.
  std::string uri = req.GetUri();
  meta_vars_["SCRIPT_NAME"] =
      path_info.HasValue() ? lib::type::Optional<std::string>(uri.substr(
                                 0, uri.length() - path_info.Value().length()))
                           : lib::type::Optional<std::string>(uri);
  // RFC 3875 4.1.14.
  meta_vars_["SERVER_NAME"] =
      lib::type::Optional<std::string>(req.GetHostName());
  // RFC 3875 4.1.15.
  meta_vars_["SERVER_PORT"] =
      lib::type::Optional<std::string>(lib::utils::ToString(req.GetHostPort()));
  // RFC 3875 4.1.16.
  meta_vars_["SERVER_PROTOCOL"] =
      lib::type::Optional<std::string>(req.GetVersion());
  // RFC 3875 4.1.17.
  meta_vars_["SERVER_SOFTWARE"] =
      lib::type::Optional<std::string>("webserv/1.0");

  script_path_ = executable_path;
}

// GET and DELETE methods are handled same. POST method requires the body to be
// passed to STDIN of the CGI script.
HttpResponse CgiExecutor::Run() {
  if (!IsScriptExtensionAllowed(script_path_, loc_.GetCgiAllowedExtensions()))
    return HttpResponse(lib::http::kForbidden);

  try {
    int pipe_in[2], pipe_out[2];

    if (pipe(pipe_in) < 0) throw std::runtime_error("pipe error");
    if (pipe(pipe_out) < 0) {
      ClosePipe(pipe_in);
      throw std::runtime_error("pipe error");
    }

    std::vector<std::string> meta_vars = GetMetaVars();
    std::vector<char*> envp = CreateEnvp(meta_vars);

    int pid = fork();
    if (pid < 0) throw std::runtime_error("fork error");

    std::string req_method = GetMetaVar("REQUEST_METHOD");

    if (pid == 0) {  // Child process
      close(pipe_in[kWriteEnd]);
      close(pipe_out[kReadEnd]);
      dup2(pipe_in[kReadEnd], STDIN_FILENO);
      dup2(pipe_out[kWriteEnd], STDOUT_FILENO);
      close(pipe_in[kReadEnd]);
      close(pipe_out[kWriteEnd]);

      char* argv[] = {const_cast<char*>(script_path_.c_str()), NULL};
      execve(script_path_.c_str(), argv, envp.data());
      exit(1);
    } else {  // Parent process
      close(pipe_in[kReadEnd]);
      close(pipe_out[kWriteEnd]);

      if (req_method == "POST") {
        write(pipe_in[kWriteEnd], body_.c_str(), body_.length());
      }
      close(pipe_in[kWriteEnd]);

      std::string cgi_output;
      // でかいデータもいい感じに取得する。epollで同じのやった気がする。
      // CGI
      // Scriptが無限ループした場合、ここでブロッキングが発生してしまうのか？
      char buffer[4096];
      ssize_t bytes_read;

      while ((bytes_read = read(pipe_out[kReadEnd], buffer, sizeof(buffer))) >
             0) {
        cgi_output.append(buffer, bytes_read);
      }
      close(pipe_out[kReadEnd]);

      int status;
      waitpid(pid, &status, 0);

      if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
        return HttpResponse(lib::http::kInternalServerError);
      }

      HttpResponse res = cgi::ParseCgiResponse(cgi_output);
      return res;
    }
  } catch (std::exception& e) {
    return HttpResponse(lib::http::kInternalServerError);
  }
}
