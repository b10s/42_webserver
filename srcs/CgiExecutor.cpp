#include "CgiExecutor.hpp"

#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>

#include <cctype>
#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <stdexcept>

#include "CgiResponseParser.hpp"
#include "HttpResponse.hpp"
#include "lib/http/Status.hpp"
#include "lib/type/Fd.hpp"
#include "lib/type/Optional.hpp"
#include "lib/utils/string_utils.hpp"
#include "socket/CgiSocket.hpp"

namespace {
std::vector<char*> CreateEnvp(const std::vector<std::string>& envs) {
  std::vector<char*> envp;
  for (size_t i = 0; i < envs.size(); ++i) {
    envp.push_back(const_cast<char*>(envs[i].c_str()));
  }
  envp.push_back(NULL);
  return envp;
}

lib::type::Optional<std::string> CreatePathInfo(
    const std::string& path, const std::vector<std::string>& extensions) {
  size_t min_pos = std::string::npos;
  size_t found_ext_len = 0;

  for (size_t i = 0; i < extensions.size(); ++i) {
    const std::string& ext = extensions[i];
    if (ext.empty()) continue;

    size_t pos = path.rfind(ext);
    while (pos != std::string::npos) {
      size_t ext_end = pos + ext.length();

      if (ext_end == path.length() || path[ext_end] == '/') {
        if (pos == 0 || path[pos - 1] != '/') {
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
ExecResult CgiExecutor::Run() {
  if (!IsScriptExtensionAllowed(script_path_, loc_.GetCgiAllowedExtensions()))
    return ExecResult(HttpResponse(lib::http::kForbidden));

  try {
    int sv[2];
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, sv) < 0) {
      throw std::runtime_error("socketpair error");
    }
    lib::type::Fd sv0(sv[0]);
    lib::type::Fd sv1(sv[1]);

    std::vector<std::string> meta_vars = GetMetaVars();
    std::vector<char*> envp = CreateEnvp(meta_vars);

    int pid = fork();
    if (pid < 0) {
      throw std::runtime_error("fork error");
    }

    std::string req_method = GetMetaVar("REQUEST_METHOD");

    if (pid == 0) {  // Child process
      sv0.Reset();
      dup2(sv1.GetFd(), STDIN_FILENO);
      dup2(sv1.GetFd(), STDOUT_FILENO);
      sv1.Reset();

      char* argv[] = {const_cast<char*>(script_path_.c_str()), NULL};
      execve(script_path_.c_str(), argv, envp.data());
      exit(1);
    } else {  // Parent process
      sv1.Reset();
      CgiSocket* cgi_socket = new CgiSocket(sv0, pid);

      if (req_method == "POST") {
        cgi_socket->Send(body_);
      }

      return ExecResult(cgi_socket);
    }
  } catch (std::exception& e) {
    return ExecResult(HttpResponse(lib::http::kInternalServerError));
  }
}
