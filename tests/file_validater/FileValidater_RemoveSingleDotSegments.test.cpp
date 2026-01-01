#include <gtest/gtest.h>

#include "FileValidator.hpp"

TEST(FileValidaterTest, RemoveSingleDotSegments_RemovesSingleDotSegments) {
  EXPECT_EQ(FileValidator::RemoveSingleDotSegments("/a/./b/c/./d"), "/a/b/c/d");
  EXPECT_EQ(FileValidator::RemoveSingleDotSegments("/./a/b/c/."), "/a/b/c");
  EXPECT_EQ(FileValidator::RemoveSingleDotSegments("/a/b/c"), "/a/b/c");
  EXPECT_EQ(FileValidator::RemoveSingleDotSegments("/./././"), "/");
  EXPECT_EQ(FileValidator::RemoveSingleDotSegments("/a/././b/././c/."), "/a/b/c");
}

TEST(FileValidaterTest, RemoveSingleDotSegments_HandlesEdgeCases) {
  EXPECT_EQ(FileValidator::RemoveSingleDotSegments("/."), "/");
  EXPECT_EQ(FileValidator::RemoveSingleDotSegments("/./"), "/");
  EXPECT_EQ(FileValidator::RemoveSingleDotSegments("/a/."), "/a");
  EXPECT_EQ(FileValidator::RemoveSingleDotSegments("/a/./"), "/a");
  EXPECT_EQ(FileValidator::RemoveSingleDotSegments("/."), "/");
  EXPECT_EQ(FileValidator::RemoveSingleDotSegments("/"), "/");
}
