#include <gtest/gtest.h>

#include <unistd.h>   // chdir, getcwd
#include <limits.h>   // PATH_MAX
#include <cstdlib>    // mkdtemp
#include <string>

#include "ConfigParser.hpp"
#include "Location.hpp"

// absolute path

static void callParseRoot(const std::string& input, Location* loc) {
  ConfigParser p;
  p.content = input;
  p.ParseRoot(loc);
}

TEST(ConfigParser, SimpleDir_ParseRoot_OK) {
  Location loc;
  EXPECT_NO_THROW(callParseRoot("/var/www/html;", &loc));
  EXPECT_EQ(loc.GetRoot(), "/var/www/html");
}

TEST(ConfigParser, SimpleDir_ParseRoot_DotdotInFileName_OK) {
  Location loc;
  EXPECT_NO_THROW(callParseRoot("/var/ww..w/html;", &loc));
  EXPECT_EQ(loc.GetRoot(), "/var/ww..w/html");
}

TEST(ConfigParser, SimpleDir_ParseRoot_MissingSemicolon_Throws) {
  Location loc;
  EXPECT_THROW(callParseRoot("/var/www/html", &loc), std::runtime_error);
}

TEST(ConfigParser, SimpleDir_ParseRoot_EmptyValue_Throws) {
  Location loc;
  EXPECT_THROW(callParseRoot(";", &loc), std::runtime_error);
}

TEST(ConfigParser, SimpleDir_ParseRoot_ExtraTokens_Throws) {
  Location loc;
  EXPECT_THROW(callParseRoot("/var /www;", &loc), std::runtime_error);
}

TEST(ConfigParser, SimpleDir_ParseRoot_InvalidChar_Throws) {
  Location loc;
  EXPECT_THROW(callParseRoot("/var/www\r/html;", &loc), std::runtime_error);
}

// relative path

// --- helper: getcwd ---
static std::string GetCwdOrThrow() {
  char buf[PATH_MAX];
  if (!getcwd(buf, sizeof(buf))) {
    throw std::runtime_error("getcwd failed in test");
  }
  return std::string(buf);
}

// --- helper: temp dir ---
static std::string MakeTempDirOrThrow() {
  char tmpl[] = "/tmp/parse_root_test.XXXXXX"; // six place holders for mkdtemp
  char* p = mkdtemp(tmpl); // creates unique temp dir every time
  if (!p) throw std::runtime_error("mkdtemp failed in test");
  return std::string(p);
}

// --- helper: RAII chdir ---
class ScopedChdir {
 public:
  explicit ScopedChdir(const std::string& dir)
      : old_(GetCwdOrThrow()), changed_(false) {
    if (chdir(dir.c_str()) != 0) {
      throw std::runtime_error("chdir failed in test");
    }
    changed_ = true;
  }
  ~ScopedChdir() {
    if (changed_) chdir(old_.c_str());  // restore old cwd
  }
 private:
  std::string old_;
  bool changed_;
};

class ConfigParserParseRootTest : public ::testing::Test {
 protected:
  ConfigParser parser;
  Location loc;

  void SetContent(const std::string& s) {
    parser.content = s;
  }
};

TEST_F(ConfigParserParseRootTest, ParseRoot_RelativePath_ResolvesAgainstCwd) {
  const std::string tmp = MakeTempDirOrThrow();
  ScopedChdir cd(tmp);
  SetContent("www;");
  parser.ParseRoot(&loc);
  EXPECT_EQ(loc.GetRoot(), tmp + "/www");
}

TEST_F(ConfigParserParseRootTest, ParseRoot_RelativePathWithDot_RelolvesAgainstCwd) {
  const std::string tmp = MakeTempDirOrThrow();
  ScopedChdir cd(tmp);
  SetContent("./www;");
  parser.ParseRoot(&loc);
  EXPECT_EQ(loc.GetRoot(), tmp + "/www");
}

TEST_F(ConfigParserParseRootTest, ParseRoot_RelativePathWithDotDot_Throws) {
  const std::string tmp = MakeTempDirOrThrow();
  ScopedChdir cd(tmp);
  SetContent("../secret;");
  EXPECT_THROW(parser.ParseRoot(&loc), std::runtime_error);
}
