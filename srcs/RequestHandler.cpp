#include "RequestHandler.hpp"
#include "HttpRequest.hpp"
#include "ServerConfig.hpp"
#include "lib/http/Method.hpp"
#include "lib/http/Status.hpp"

#include <iostream>

RequestHandler::RequestHandler() {}

RequestHandler::RequestHandler(ServerConfig conf, HttpRequest req): conf_(conf), req_(req) {
  full_path_ = conf.GetLocations()[0].GetRoot() + req.GetUri();
}

RequestHandler::~RequestHandler() {}

HttpResponse RequestHandler::Run() {
  lib::http::Method method = req_.GetMethod();
  if (method == lib::http::kGet) {
    HandleGet();
  } else if (method == lib::http::kPost) {
  } else if (method == lib::http::kDelete) {
  } else {}
  return res_;
}

void RequestHandler::HandleGet() {
  
  res_.SetStatus(lib::http::kOk, lib::http::StatusToString(lib::http::kOk));
  res_.AddHeader("Content-Type", "text/plain");
  res_.SetBody("Hello, world!");
}

void RequestHandler::HandlePost() {}
void RequestHandler::HandleDelete() {}

}

