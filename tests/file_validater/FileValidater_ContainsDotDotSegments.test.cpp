#include <gtest/gtest.h>

#include "FileValidater.hpp"

TEST(FileValidatorTest,ContainsDotDotSegments_RejectsDotDotSegments) {
  EXPECT_TRUE(FileValidator::ContainsDotDotSegments("/../a"));
  EXPECT_TRUE(FileValidator::ContainsDotDotSegments("/a/../b"));
  EXPECT_TRUE(FileValidator::ContainsDotDotSegments("/a/.."));
  EXPECT_TRUE(FileValidator::ContainsDotDotSegments(".."));
  EXPECT_TRUE(FileValidator::ContainsDotDotSegments("../a"));
}

TEST(FileValidatorTest,ContainsDotDotSegments_AcceptsSafePaths) {
  EXPECT_FALSE(FileValidator::ContainsDotDotSegments("/a..b"));
  EXPECT_FALSE(FileValidator::ContainsDotDotSegments("/a/..b"));
  EXPECT_FALSE(FileValidator::ContainsDotDotSegments("/a/b.."));
  EXPECT_FALSE(FileValidator::ContainsDotDotSegments("/a/b/c"));
  EXPECT_FALSE(FileValidator::ContainsDotDotSegments("/"));
  EXPECT_FALSE(FileValidator::ContainsDotDotSegments("/a/b/./c"));
}