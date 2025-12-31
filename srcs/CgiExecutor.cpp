#include "CgiExecutor.hpp"

#include <sys/wait.h>

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
lib::type::Optional<std::string> ReadShebang(const std::string& file_path) {
  std::ifstream file(file_path.c_str());
  std::string line;

  if (file.is_open()) {
    std::getline(file, line);
  }

  if (line.length() < 2 || line[0] != '#' || line[1] != '!') {
    return lib::type::Optional<std::string>();
  }

  // skip "#!"
  std::string script_path = line.substr(2);

  std::string::size_type start_pos = 0;
  while (start_pos < script_path.length() &&
         std::isspace(static_cast<unsigned char>(script_path[start_pos]))) {
    start_pos++;
  }

  return lib::type::Optional<std::string>(script_path.substr(start_pos));
}

std::vector<char*> CreateEnvp(const std::vector<std::string>& envs) {
  std::vector<char*> envp;
  for (size_t i = 0; i < envs.size(); ++i) {
    envp.push_back(const_cast<char*>(envs[i].c_str()));
  }
  envp.push_back(NULL);
  return envp;
}

void PrintEnv(const std::vector<std::string>& envs) {
  for (size_t i = 0; i < envs.size(); ++i) {
    std::cout << envs[i] << std::endl;
  }
}

void ClosePipe(int fd[2]) {
  close(fd[0]);
  close(fd[1]);
}
}  // namespace

CgiExecutor::CgiExecutor(const HttpRequest& req,
                         const std::string& script_path) {
  InitializeMetaVars(req);
  script_path_ = script_path;
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
  meta_vars_["PATH_INFO"] = lib::type::Optional<std::string>();
  // RFC 3875 4.1.6.
  meta_vars_["PATH_TRANSLATED"] = lib::type::Optional<std::string>();
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
  meta_vars_["SCRIPT_NAME"] = lib::type::Optional<std::string>(req.GetUri());
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
}

HttpResponse CgiExecutor::Run() {
  PrintEnv(GetMetaVars());
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

    if (pid == 0) {  // Child process
      close(pipe_in[kWriteEnd]);
      close(pipe_out[kReadEnd]);
      dup2(pipe_in[kReadEnd], STDIN_FILENO);
      dup2(pipe_out[kWriteEnd], STDOUT_FILENO);
      close(pipe_in[kReadEnd]);
      close(pipe_out[kWriteEnd]);

      lib::type::Optional<std::string> shebang = ReadShebang(script_path_);
      char* argv[] = {const_cast<char*>(script_path_.c_str()), NULL};
      execve(script_path_.c_str(), argv, envp.data());
      std::cout << "Status: 500 Internal Server Error\r\nContent-Type: "
                   "text/plain\r\n\r\n";
      std::cout << "Execve failed: " << std::strerror(errno) << std::endl;
      exit(1);
    } else {  // Parent process
      close(pipe_in[kReadEnd]);
      std::string req_method = GetMetaVar("REQUEST_METHOD");
      if (req_method == "GET") {
        close(pipe_out[kWriteEnd]);
      } else if (req_method == "POST") {
        ;
      } else if (req_method == "DELETE") {
        ;
      }

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
      ClosePipe(pipe_out);

      int status;
      waitpid(pid, &status, 0);

      std::cout << "cgi output: " << cgi_output << std::endl;
      HttpResponse res = cgi::ParseCgiResponse(cgi_output);
      return res;
    }
  } catch (std::exception& e) {
    HttpResponse res(lib::http::kInternalServerError);
    return res;
  }
}
