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

    // ConfigParser に食わせる設定（あなたの文法に合わせて調整してね）
    // 例: { listen 8081; location / { root <tmp_>; index index.html; methods POST; } }
    ConfigParser parser;
    parser.content =
        "{ "
        "listen 0.0.0.0:8081; "
        "location / { "
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
  }
};

TEST_F(RequestHandlerPostTest, StaticPost_CreatesFile_AndReturns201) {
  HttpRequest req;
  req.SetMethod(lib::http::kPost);
  req.SetUri("/text.txt");
  req.SetBufferForTest("hello post world");
  req.SetContentLengthForTest(16);          // 16 bytes
  ASSERT_TRUE(req.AdvanceBody());           // set body_

  RequestHandler handler(config_, req);
  ExecResult r = handler.Run();

  EXPECT_EQ(r.response.GetStatus(), lib::http::kCreated);
  lib::type::Optional<std::string> loc_header =
      r.response.GetHeader("location");
  ASSERT_TRUE(loc_header.HasValue());
  EXPECT_EQ(loc_header.Value(), "/text.txt");

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
  req.SetUri("/dir/");          // ← trailing slash
  req.SetBufferForTest("x");

  RequestHandler handler(config_, req);
  ExecResult r = handler.Run();

  EXPECT_EQ(r.response.GetStatus(), lib::http::kBadRequest);
}

TEST_F(RequestHandlerPostTest, StaticPost_TargetIsDirectory_Returns400) {
  MakeDir(tmp_ + "/dir", 0755);

  HttpRequest req;
  req.SetMethod(lib::http::kPost);
  req.SetUri("/dir");
  req.SetBufferForTest("x");

  RequestHandler handler(config_, req);
  ExecResult r = handler.Run();

  EXPECT_EQ(r.response.GetStatus(), lib::http::kBadRequest);
}

TEST_F(RequestHandlerPostTest, StaticPost_NoWritePermission_Returns403) {
  chmod(tmp_.c_str(), 0555);  // 読み取り/実行のみ

  HttpRequest req;
  req.SetMethod(lib::http::kPost);
  req.SetUri("/cant.txt");
  req.SetBufferForTest("x");

  RequestHandler handler(config_, req);
  ExecResult r = handler.Run();

  EXPECT_EQ(r.response.GetStatus(), lib::http::kForbidden);

  // 後始末（他テストに影響しないように戻す）
  chmod(tmp_.c_str(), 0755);
}
