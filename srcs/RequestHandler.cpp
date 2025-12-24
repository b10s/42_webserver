#include "RequestHandler.hpp"

#include <iostream>

#include "HttpRequest.hpp"
#include "ServerConfig.hpp"
#include "lib/http/Method.hpp"
#include "lib/http/Status.hpp"
#include "lib/utils/ReadFile.hpp"

RequestHandler::RequestHandler() {
}

RequestHandler::RequestHandler(ServerConfig conf, HttpRequest req)
    : conf_(conf), req_(req) {
  // very simple file path sample
  full_path_ = conf.GetLocations()[0].GetRoot() + req.GetUri() + conf.GetLocations()[0].GetIndexFiles()[0];
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

void RequestHandler::HandleGet() {
  std::string body = lib::utils::ReadFile(full_path_);
  res_.AddHeader("Content-Type", "text/html");
  res_.SetBody(body);
  res_.SetStatus(lib::http::kOk);
}

void RequestHandler::HandlePost() {
}

void RequestHandler::HandleDelete() {
}
