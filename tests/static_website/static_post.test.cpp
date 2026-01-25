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

static std::string MakeTempDir() {
  char tmpl[] = "/tmp/webserv_post_test.XXXXXX";
  char* p = mkdtemp(tmpl);
  if (!p) return "";
  return std::string(p);
}

class RequestHandlerPostTest : public ::testing::Test {
 protected:
  std::string tmp_;
  ServerConfig config_;

  void SetUp() override {
    tmp_ = MakeTempDir();
    ASSERT_FALSE(tmp_.empty());

    ConfigParser parser;
    parser.content =
        "{ "
        "listen 0.0.0.0:8081; "
        "location /upload { "
        "  root " + tmp_ + "; "
        "  index index.html; "
        "  allowed_methods POST; "
        "  cgi off; "
        "} "
        "}";
    ASSERT_NO_THROW(parser.ParseServer());

    const std::vector<ServerConfig>& servers = parser.GetServerConfigs();
    ASSERT_EQ(servers.size(), 1u);

    config_ = servers[0]; // copy
  }

  void TearDown() override {
    unlink((tmp_ + "/text.txt").c_str());
    unlink((tmp_ + "/exist.txt").c_str());
    unlink((tmp_ + "/cant.txt").c_str());
    rmdir((tmp_ + "/dir").c_str());
    rmdir(tmp_.c_str());
  }
};

TEST_F(RequestHandlerPostTest, StaticPost_CreatesFile_AndReturns201) {
  HttpRequest req;
  req.SetMethod(lib::http::kPost);
  req.SetUri("/upload/text.txt");
  req.SetBufferForTest("hello post world");
  req.SetContentLengthForTest(16);          // 16 bytes
  ASSERT_TRUE(req.AdvanceBody());           // set body_

  RequestHandler handler(config_, req);
  ExecResult r = handler.Run();

  EXPECT_EQ(r.response.GetStatus(), lib::http::kCreated);
  lib::type::Optional<std::string> loc_header =
      r.response.GetHeader("location");
  ASSERT_TRUE(loc_header.HasValue());
  EXPECT_EQ(loc_header.Value(), "/upload/text.txt");

  // response body should be empty
  lib::type::Optional<std::string> body = r.response.GetBody();
  ASSERT_TRUE(body.HasValue());
  EXPECT_EQ(body.Value(), "");

  const std::string saved = tmp_ + "/text.txt";
  EXPECT_EQ(ReadAll(saved), "hello post world");
}

TEST_F(RequestHandlerPostTest, StaticPost_UriEndsWithSlash_Returns400) {
  HttpRequest req;
  req.SetMethod(lib::http::kPost);
  req.SetUri("/upload/dir/");          // ← trailing slash
  req.SetBufferForTest("x");

  RequestHandler handler(config_, req);
  ExecResult r = handler.Run();

  EXPECT_EQ(r.response.GetStatus(), lib::http::kBadRequest);
}

TEST_F(RequestHandlerPostTest, StaticPost_TargetIsDirectory_Returns400) {
  MakeDir(tmp_ + "/dir", 0755);

  HttpRequest req;
  req.SetMethod(lib::http::kPost);
  req.SetUri("/upload/dir");
  req.SetBufferForTest("x");

  RequestHandler handler(config_, req);
  ExecResult r = handler.Run();

  EXPECT_EQ(r.response.GetStatus(), lib::http::kBadRequest);
}

TEST_F(RequestHandlerPostTest, StaticPost_NoWritePermission_Returns403) {
  chmod(tmp_.c_str(), 0555);  // read & execute only

  HttpRequest req;
  req.SetMethod(lib::http::kPost);
  req.SetUri("/upload/cant.txt");
  req.SetBufferForTest("x");

  RequestHandler handler(config_, req);
  ExecResult r = handler.Run();

  EXPECT_EQ(r.response.GetStatus(), lib::http::kForbidden);

  chmod(tmp_.c_str(), 0755);
}

// POST to existing file returns chosen behavior (409 or overwrite behavior) ?
// for now, we implement overwrite behavior
TEST_F(RequestHandlerPostTest, StaticPost_OverwriteExistingFile_Succeeds) {
  const std::string filepath = tmp_ + "/exist.txt";
  {
    std::ofstream ofs(filepath.c_str(), std::ios::binary);
    ofs << "original content";
  }

  HttpRequest req;
  req.SetMethod(lib::http::kPost);
  req.SetUri("/upload/exist.txt");
  req.SetBufferForTest("new content");
  req.SetContentLengthForTest(11);          // 11 bytes
  ASSERT_TRUE(req.AdvanceBody());           // set body_

  RequestHandler handler(config_, req);
  ExecResult r = handler.Run();

  EXPECT_EQ(r.response.GetStatus(), lib::http::kCreated);

  const std::string saved = tmp_ + "/exist.txt";
  EXPECT_EQ(ReadAll(saved), "new content");
} 
