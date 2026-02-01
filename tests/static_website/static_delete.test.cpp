#include <gtest/gtest.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <string>

#include "ConfigParser.hpp"
#include "ServerConfig.hpp"
#include "Location.hpp"
#include "RequestHandler.hpp"
#include "HttpRequest.hpp"
#include "lib/http/Method.hpp"
#include "lib/http/Status.hpp"
#include "lib/utils/file_utils.hpp"
#include "lib/exception/ResponseStatusException.hpp"
#include "lib/type/Optional.hpp"

static std::string ReadAll(const std::string& path) {
  std::ifstream ifs(path.c_str(), std::ios::binary);
  std::string s((std::istreambuf_iterator<char>(ifs)),
                 std::istreambuf_iterator<char>());
  return s;
}

static void MakeDir(const std::string& path, mode_t mode) {
  mkdir(path.c_str(), mode);
}

static void WriteFile(const std::string& path, const std::string& body) {
  std::ofstream ofs(path.c_str(), std::ios::binary);
  ofs << body;
}

static bool PathExists(const std::string& path) {
  struct stat st;
  return ::stat(path.c_str(), &st) == 0;
}

static std::string MakeTempDir() {
  char tmpl[] = "/tmp/webserv_delete_test.XXXXXX";
  char* p = mkdtemp(tmpl);
  if (!p) return "";
  return std::string(p);
}

class RequestHandlerDeleteTest : public ::testing::Test {
 protected:
  std::string tmp_;
  ServerConfig config_;

  void SetUp() override {
    tmp_ = MakeTempDir();
    ASSERT_FALSE(tmp_.empty());

    // /upload -> tmp_
    ConfigParser parser;
    parser.content =
        "{ "
        "listen 0.0.0.0:8082; "
        "location /upload { "
        "  root " + tmp_ + "; "
        "  index index.html; "
        "  allowed_methods DELETE; "
        "  cgi off; "
        "} "
        "}";
    ASSERT_NO_THROW(parser.ParseServer());

    const std::vector<ServerConfig>& servers = parser.GetServerConfigs();
    ASSERT_EQ(servers.size(), 1u);

    config_ = servers[0]; // copy
  }

  void TearDown() override {
    // best-effort cleanup
    unlink((tmp_ + "/victim.txt").c_str());
    unlink((tmp_ + "/cant_delete.txt").c_str());
    unlink((tmp_ + "/keep.txt").c_str());
    rmdir((tmp_ + "/dir").c_str());
    rmdir(tmp_.c_str());
  }

  ExecResult RunDelete(const std::string& uri) {
    HttpRequest req;
    req.SetMethod(lib::http::kDelete);
    req.SetUri(uri);

    RequestHandler handler(config_, req);
    return handler.Run();
  }
};

// ======================== happy path ========================
TEST_F(RequestHandlerDeleteTest, StaticDelete_DeletesFile_AndReturns200or204) {
  const std::string victim = tmp_ + "/victim.txt";
  WriteFile(victim, "bye");
  ASSERT_TRUE(PathExists(victim));

  ExecResult r = RunDelete("/upload/victim.txt");

  // In this project, return 200 OK (not 204 No Content)
  const lib::http::Status st = r.response.GetStatus();
  EXPECT_TRUE(st == lib::http::kOk);

  EXPECT_FALSE(PathExists(victim));
  lib::type::Optional<std::string> body = r.response.GetBody();
  ASSERT_TRUE(body.HasValue());
  EXPECT_EQ(body.Value(), "File deleted successfully");
}

// ======================== error cases ========================

// delete non-existing file -> 404
TEST_F(RequestHandlerDeleteTest, StaticDelete_FileNotFound_Returns404) {
  ExecResult r = RunDelete("/upload/no_such_file.txt");
  EXPECT_EQ(r.response.GetStatus(), lib::http::kNotFound);
}

// delete a directory -> (recommended) 403 or 400/409 depending on your policy
TEST_F(RequestHandlerDeleteTest, StaticDelete_TargetIsDirectory_ReturnsClientError) {
  MakeDir(tmp_ + "/dir", 0755);
  ASSERT_TRUE(PathExists(tmp_ + "/dir"));

  ExecResult r = RunDelete("/upload/dir");

  // Accept common policies:
  // - 403 Forbidden: "we don't allow deleting directories"
  // - 400 Bad Request: "invalid delete target"
  // - 409 Conflict: "cannot delete directory"
  const lib::http::Status st = r.response.GetStatus();
  EXPECT_TRUE(st == lib::http::kForbidden ||
              st == lib::http::kBadRequest);
}

// delete forbidden due to permissions -> 403
TEST_F(RequestHandlerDeleteTest, StaticDelete_NoPermission_Returns403) {
  const std::string target = tmp_ + "/cant_delete.txt";
  WriteFile(target, "protected");
  ASSERT_TRUE(PathExists(target));

  // Make directory non-writable so unlink/remove should fail (typical POSIX rule)
  chmod(tmp_.c_str(), 0555);

  ExecResult r = RunDelete("/upload/cant_delete.txt");

  EXPECT_EQ(r.response.GetStatus(), lib::http::kForbidden);

  // restore permissions for cleanup
  chmod(tmp_.c_str(), 0755);
  // File should still exist
  EXPECT_TRUE(PathExists(target));
}

// method not allowed (DELETE not allowed) -> 405
TEST_F(RequestHandlerDeleteTest, StaticDelete_MethodNotAllowed_Returns405) {
  // Build another config that allows only GET (or POST) to verify 405 works
  ConfigParser parser;
  parser.content =
      "{ "
      "listen 0.0.0.0:8083; "
      "location /upload { "
      "  root " + tmp_ + "; "
      "  index index.html; "
      "  allowed_methods GET; "
      "  cgi off; "
      "} "
      "}";
  ASSERT_NO_THROW(parser.ParseServer());
  ServerConfig conf = parser.GetServerConfigs()[0];

  HttpRequest req;
  req.SetMethod(lib::http::kDelete);
  req.SetUri("/upload/keep.txt");

  RequestHandler handler(conf, req);
  ExecResult r = handler.Run();

  EXPECT_EQ(r.response.GetStatus(), lib::http::kMethodNotAllowed);
}

// (optional) trailing slash should probably be treated as directory-ish.
// If your policy is: reject -> 400/403/409; or treat as not found -> 404; accept several.
TEST_F(RequestHandlerDeleteTest, StaticDelete_UriEndsWithSlash_ReturnsClientErrorOr404) {
  MakeDir(tmp_ + "/dir", 0755);

  ExecResult r = RunDelete("/upload/dir/");  // trailing slash

  const lib::http::Status st = r.response.GetStatus();
  EXPECT_TRUE(st == lib::http::kForbidden ||
              st == lib::http::kBadRequest ||
              st == lib::http::kNotFound);
}
