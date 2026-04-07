#include <gtest/gtest.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <string>
#include <vector>

#include "ConfigParser.hpp"
#include "ServerConfig.hpp"
#include "Location.hpp"
#include "RequestHandler.hpp"
#include "HttpRequest.hpp"
#include "lib/http/Method.hpp"
#include "lib/http/Status.hpp"
#include "lib/type/Optional.hpp"

static std::string ReadAll(const std::string& path) {
  std::ifstream ifs(path.c_str(), std::ios::binary);
  return std::string((std::istreambuf_iterator<char>(ifs)),
                     std::istreambuf_iterator<char>());
}

static void WriteFile(const std::string& path, const std::string& content) {
  std::ofstream ofs(path.c_str(), std::ios::binary);
  ofs << content;
}

static void MakeDir(const std::string& path, mode_t mode) {
  mkdir(path.c_str(), mode);
}

static std::string MakeTempDir() {
  char tmpl[] = "/tmp/webserv_get_test.XXXXXX";
  char* p = mkdtemp(tmpl);
  if (!p) return "";
  return std::string(p);
}

class RequestHandlerGetTest : public ::testing::Test {
 protected:
  std::string tmp_;
  ServerConfig config_;

  void SetUp() override {
    tmp_ = MakeTempDir();
    ASSERT_FALSE(tmp_.empty());

    MakeDir((tmp_ + "/dir").c_str(), 0755);
    MakeDir((tmp_ + "/auto").c_str(), 0755);

    WriteFile(tmp_ + "/hello.txt", "hello get");
    WriteFile(tmp_ + "/dir/index.html", "<html>index page</html>");
    WriteFile(tmp_ + "/auto/one.txt", "one");
    WriteFile(tmp_ + "/auto/two.txt", "two");

    ConfigParser parser;
    parser.content =
        "{ "
        "listen 0.0.0.0:8081; "
        "location / { "
        "  root " + tmp_ + "; "
        "  index index.html; "
        "  allowed_methods GET; "
        "  cgi off; "
        "} "
        "location /auto { "
        "  root " + tmp_ + "/auto; "
        "  allowed_methods GET; "
        "  autoindex on; "
        "  cgi off; "
        "} "
        "}";
    ASSERT_NO_THROW(parser.ParseServer());

    const std::vector<ServerConfig>& servers = parser.GetServerConfigs();
    ASSERT_EQ(servers.size(), 1u);
    config_ = servers[0];
  }

  void TearDown() override {
    unlink((tmp_ + "/hello.txt").c_str());
    unlink((tmp_ + "/dir/index.html").c_str());
    unlink((tmp_ + "/auto/one.txt").c_str());
    unlink((tmp_ + "/auto/two.txt").c_str());
    rmdir((tmp_ + "/dir").c_str());
    rmdir((tmp_ + "/auto").c_str());
    rmdir(tmp_.c_str());
  }
};

TEST_F(RequestHandlerGetTest, StaticGet_ExistingFile_Returns200AndBody) {
  HttpRequest req;
  req.SetMethod(lib::http::kGet);
  req.SetUri("/hello.txt");

  RequestHandler handler(config_, req);
  ExecResult r = handler.Run();

  EXPECT_EQ(r.response.GetStatus(), lib::http::kOk);

  lib::type::Optional<std::string> body = r.response.GetBody();
  ASSERT_TRUE(body.HasValue());
  EXPECT_EQ(body.Value(), "hello get");

  lib::type::Optional<std::string> content_type =
      r.response.GetHeader("content-type");
  ASSERT_TRUE(content_type.HasValue());
}

// directory + trailing slash -> serve index file
TEST_F(RequestHandlerGetTest, StaticGet_DirectoryWithSlash_ReturnsIndexFile) {
  HttpRequest req;
  req.SetMethod(lib::http::kGet);
  req.SetUri("/dir/");

  RequestHandler handler(config_, req);
  ExecResult r = handler.Run();

  EXPECT_EQ(r.response.GetStatus(), lib::http::kOk);

  lib::type::Optional<std::string> body = r.response.GetBody();
  ASSERT_TRUE(body.HasValue());
  EXPECT_EQ(body.Value(), "<html>index page</html>");
}

// directory without trailing slash -> 301 Moved Permanently (redirect to URI with slash)
TEST_F(RequestHandlerGetTest, StaticGet_DirectoryWithoutSlash_Returns301) {
  HttpRequest req;
  req.SetMethod(lib::http::kGet);
  req.SetUri("/dir");

  RequestHandler handler(config_, req);
  ExecResult r = handler.Run();

  EXPECT_EQ(r.response.GetStatus(), lib::http::kMovedPermanently);

  lib::type::Optional<std::string> loc =
      r.response.GetHeader("location");
  ASSERT_TRUE(loc.HasValue());
  EXPECT_EQ(loc.Value(), "/dir/");
}

// autoindex enabled
TEST_F(RequestHandlerGetTest, StaticGet_AutoIndexOn_ReturnsDirectoryListing) {
  HttpRequest req;
  req.SetMethod(lib::http::kGet);
  req.SetUri("/auto/");

  RequestHandler handler(config_, req);
  ExecResult r = handler.Run();

  EXPECT_EQ(r.response.GetStatus(), lib::http::kOk);

  lib::type::Optional<std::string> body = r.response.GetBody();
  ASSERT_TRUE(body.HasValue());

  EXPECT_NE(body.Value().find("Index of /auto/"), std::string::npos);
  EXPECT_NE(body.Value().find("one.txt"), std::string::npos);
  EXPECT_NE(body.Value().find("two.txt"), std::string::npos);

  lib::type::Optional<std::string> content_type =
      r.response.GetHeader("content-type");
  ASSERT_TRUE(content_type.HasValue());
  EXPECT_EQ(content_type.Value(), "text/html");
}

TEST_F(RequestHandlerGetTest, StaticGet_NotFound_Returns404) {
  HttpRequest req;
  req.SetMethod(lib::http::kGet);
  req.SetUri("/no_such_file.txt");

  RequestHandler handler(config_, req);
  ExecResult r = handler.Run();

  EXPECT_EQ(r.response.GetStatus(), lib::http::kNotFound);
}
