#include <gtest/gtest.h>

#include "FileValidater.hpp"

TEST(FileValidaterTest,ContainsDotDotSegments_RejectsDotDotSegments) {
  EXPECT_TRUE(FileValidater::ContainsDotDotSegments("/../a"));
  EXPECT_TRUE(FileValidater::ContainsDotDotSegments("/a/../b"));
  EXPECT_TRUE(FileValidater::ContainsDotDotSegments("/a/.."));
  EXPECT_TRUE(FileValidater::ContainsDotDotSegments(".."));
  EXPECT_TRUE(FileValidater::ContainsDotDotSegments("../a"));
}

TEST(FileValidaterTest,ContainsDotDotSegments_AcceptsSafePaths) {
  EXPECT_FALSE(FileValidater::ContainsDotDotSegments("/a..b"));
  EXPECT_FALSE(FileValidater::ContainsDotDotSegments("/a/..b"));
  EXPECT_FALSE(FileValidater::ContainsDotDotSegments("/a/b.."));
  EXPECT_FALSE(FileValidater::ContainsDotDotSegments("/a/b/c"));
  EXPECT_FALSE(FileValidater::ContainsDotDotSegments("/"));
  EXPECT_FALSE(FileValidater::ContainsDotDotSegments("/a/b/./c"));
}