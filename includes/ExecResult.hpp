#ifndef EXECRESULT_HPP_
#define EXECRESULT_HPP_

#include "HttpResponse.hpp"

class ASocket;

struct ExecResult {
  HttpResponse response;
  ASocket* new_socket;
  bool is_async;

  ExecResult()
      : response(lib::http::kInternalServerError),
        new_socket(NULL),
        is_async(false) {
  }

  explicit ExecResult(const HttpResponse& res)
      : response(res), new_socket(NULL), is_async(false) {
  }

  explicit ExecResult(ASocket* sock)
      : response(lib::http::kOk), new_socket(sock), is_async(true) {
  }
};

#endif
