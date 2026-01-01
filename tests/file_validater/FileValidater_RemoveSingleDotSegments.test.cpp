#include <gtest/gtest.h>

#include "FileValidater.hpp"

TEST(FileValidaterTest, RemoveSingleDotSegments_RemovesSingleDotSegments) {
  EXPECT_EQ(FileValidater::RemoveSingleDotSegments("/a/./b/c/./d"), "/a/b/c/d");
  EXPECT_EQ(FileValidater::RemoveSingleDotSegments("/./a/b/c/."), "/a/b/c");
  EXPECT_EQ(FileValidater::RemoveSingleDotSegments("/a/b/c"), "/a/b/c");
  EXPECT_EQ(FileValidater::RemoveSingleDotSegments("/./././"), "/");
  EXPECT_EQ(FileValidater::RemoveSingleDotSegments("/a/././b/././c/."), "/a/b/c");
}

TEST(FileValidaterTest, RemoveSingleDotSegments_HandlesEdgeCases) {
  EXPECT_EQ(FileValidater::RemoveSingleDotSegments("/."), "/");
  EXPECT_EQ(FileValidater::RemoveSingleDotSegments("/./"), "/");
  EXPECT_EQ(FileValidater::RemoveSingleDotSegments("/a/."), "/a");
  EXPECT_EQ(FileValidater::RemoveSingleDotSegments("/a/./"), "/a");
  EXPECT_EQ(FileValidater::RemoveSingleDotSegments("/."), "/");
  EXPECT_EQ(FileValidater::RemoveSingleDotSegments("/"), "/");
}