#include <gtest/gtest.h>
#include "lib/http/MimeType.hpp"

// Test cases for lib::http::DetectMimeTypeFromPath function

TEST(MimeTest, DetectMimeTypeCaseInsensitive) {
  EXPECT_EQ(lib::http::DetectMimeTypeFromPath("INDEX.HTML"), "text/html");
  EXPECT_EQ(lib::http::DetectMimeTypeFromPath("Style.CsS"), "text/css");
  EXPECT_EQ(lib::http::DetectMimeTypeFromPath("SCRIPT.Js"), "application/javascript");
  EXPECT_EQ(lib::http::DetectMimeTypeFromPath("Image.PnG"), "image/png");
}

TEST(MimeTest, DetectMimeTypeNoExtension) {
  EXPECT_EQ(lib::http::DetectMimeTypeFromPath("file_without_extension"), "application/octet-stream");
}

TEST(MimeTest, DetectMimeTypeEmptyString) {
  EXPECT_EQ(lib::http::DetectMimeTypeFromPath(""), "application/octet-stream");
}

TEST(MimeTest, DetectMimeTypeUnknownExtension) {
  EXPECT_EQ(lib::http::DetectMimeTypeFromPath("archive.unknownext"), "application/octet-stream");
}

