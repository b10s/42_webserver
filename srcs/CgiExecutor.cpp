#include "CgiExecutor.hpp"

#include <sys/wait.h>

#include <cctype>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <iostream>

#include "lib/http/Status.hpp"
#include "lib/type/Optional.hpp"
#include "lib/utils/string_utils.hpp"

namespace {
std::string CreateQueryString(const Dict& queries) {
  if (queries.empty()) {
    return "";
  }
  std::string query_string;

  for (Dict::const_iterator it = queries.begin(); it != queries.end(); ++it) {
    if (it != queries.begin()) {
      query_string += "&";
    }
    query_string += it->first;
    if (!it->second.empty()) {
      query_string += "=" + it->second;
    }
  }

  return query_string;
}

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

void ClosePipe(int fd[2]) {
  close(fd[0]);
  close(fd[1]);
}

HttpResponse ParseCgiResponse(const std::string& cgi_output) {
  HttpResponse response;

  // Find separation between headers and body
  size_t header_end = cgi_output.find("\r\n\r\n");
  if (header_end == std::string::npos) {
    header_end = cgi_output.find("\n\n");
  }

  if (header_end == std::string::npos) {
    // RFC 3875 Violation: No header-body separator
    response.SetStatus(lib::http::kInternalServerError);
    response.SetBody(
        "CGI output missing header-body separator. Must conform to RFC 3875.");
    return response;
  }

  std::string headers_part = cgi_output.substr(0, header_end);
  response.SetBody(cgi_output.substr(
      header_end + ((cgi_output[header_end] == '\r') ? 4 : 2)));

  std::stringstream ss(headers_part);
  std::string line;
  while (std::getline(ss, line)) {
    if (!line.empty() && line[line.length() - 1] == '\r')
      line.erase(line.length() - 1);
    if (line.empty()) continue;

    size_t colon = line.find(':');
    if (colon != std::string::npos) {
      std::string key = line.substr(0, colon);
      std::string val = line.substr(colon + 1);

      size_t val_start = 0;
      while (val_start < val.length() && std::isspace(val[val_start]))
        val_start++;
      val = val.substr(val_start);

      if (key == "Status") {
        size_t space_pos = val.find(' ');
        std::string status_str;
        std::string reason_phrase;
        if (space_pos != std::string::npos) {
          status_str = val.substr(0, space_pos);
          reason_phrase = val.substr(space_pos + 1);
        } else {
          status_str = val;
          reason_phrase = "";
        }
        lib::type::Optional<long> status_code_opt =
            lib::utils::StrToLong(status_str);
        if (status_code_opt.HasValue()) {
          response.SetStatus(
              static_cast<lib::http::Status>(status_code_opt.Value()));
        } else {
          response.SetStatus(lib::http::kInternalServerError);
          response.SetBody("CGI output contains malformed Status header: " +
                           val);
          return response;
        }
      } else {
        response.AddHeader(key, val);
      }
    } else {
      response.SetStatus(lib::http::kInternalServerError);
      response.SetBody("CGI output contains malformed header line: " + line);
      return response;
    }
  }
  return response;
}
}  // namespace

CgiExecutor::CgiExecutor(const HttpRequest& req, const std::string& script_path){
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
  meta_vars_["AUTH_TYPE"] = lib::type::Optional<std::string>();
  // RFC 3875 4.1.2.
  //  meta_vars_["CONTENT_LENGTH"] = req.GetHeader("content-length");
  meta_vars_["CONTENT_LENGTH"] = lib::type::Optional<std::string>();
  // RFC 3875 4.1.3.
  //  meta_vars_["CONTENT_TYPE"] = req.GetHeader("content-type");
  meta_vars_["CONTENT_TYPE"] = lib::type::Optional<std::string>();
  // RFC 3875 4.1.4.
  meta_vars_["GATEWAY_INTERFACE"] = lib::type::Optional<std::string>("CGI/1.1");
  // RFC 3875 4.1.5.
  meta_vars_["PATH_INFO"] = lib::type::Optional<std::string>();
  // RFC 3875 4.1.6.
  meta_vars_["PATH_TRANSLATED"] =
      lib::type::Optional<std::string>(script_path_);
  // RFC 3875 4.1.7.
  meta_vars_["QUERY_STRING"] =
      lib::type::Optional<std::string>(CreateQueryString(req.GetQuery()));
  // RFC 3875 4.1.8.
  meta_vars_["REMOTE_ADDR"] = lib::type::Optional<std::string>();
  // RFC 3875 4.1.9.
  meta_vars_["REMOTE_HOST"] = lib::type::Optional<std::string>();
  // RFC 3875 4.1.10.
  meta_vars_["REMOTE_IDENT"] = lib::type::Optional<std::string>();
  // RFC 3875 4.1.11.
  meta_vars_["REMOTE_USER"] = lib::type::Optional<std::string>();
  // RFC 3875 4.1.12.
  meta_vars_["REQUEST_METHOD"] = lib::type::Optional<std::string>(
      lib::http::MethodToString(req.GetMethod()));
  // RFC 3875 4.1.13.
  meta_vars_["SCRIPT_NAME"] = lib::type::Optional<std::string>(req.GetUri());
  // RFC 3875 4.1.14.
  meta_vars_["SERVER_NAME"] = req.GetHostName();
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
  int pipe_in[2], pipe_out[2];

  if (pipe(pipe_in) < 0)
    // throw error
    ;
  if (pipe(pipe_out) < 0) {
    ClosePipe(pipe_in);
    // throw error
  }

  std::vector<std::string> meta_vars = GetMetaVars();
  std::vector<char*> envp = CreateEnvp(meta_vars);

  int pid = fork();
  if (pid < 0)
    ;

  if (pid == 0) {  // Child process
    close(pipe_in[kWriteEnd]);
    close(pipe_out[kReadEnd]);
    dup2(pipe_in[kReadEnd], STDIN_FILENO);
    dup2(pipe_out[kWriteEnd], STDOUT_FILENO);
    close(pipe_in[kReadEnd]);
    close(pipe_out[kWriteEnd]);

    lib::type::Optional<std::string> shebang = ReadShebang(script_path_);
    if (shebang.HasValue()) {
      std::string interpreter = shebang.Value();
      char* argv[] = {const_cast<char*>(interpreter.c_str()),
                      const_cast<char*>(script_path_.c_str()), NULL};
      execve(interpreter.c_str(), argv, envp.data());
    } else {
      char* argv[] = {const_cast<char*>(script_path_.c_str()), NULL};
      execve(script_path_.c_str(), argv, envp.data());
    }
    exit(1);
  } else {  // Parent process
    close(pipe_out[kWriteEnd]);
    close(pipe_in[kReadEnd]);

    std::string req_method = GetMetaVar("REQUEST_METHOD");
    if (req_method == "GET") {
      close(pipe_in[kWriteEnd]);
    } else if (req_method == "POST") {
      ;
    } else if (req_method == "DELETE") {
      ;
    }

    std::string cgi_output;
    char buffer[4096];
    ssize_t bytes_read;

    while ((bytes_read = read(pipe_out[kReadEnd], buffer, sizeof(buffer))) >
           0) {
      cgi_output.append(buffer, bytes_read);
    }
    ClosePipe(pipe_out);

    int status;
    waitpid(pid, &status, 0);

    HttpResponse res = ParseCgiResponse(cgi_output);
    return res;
  }
}
