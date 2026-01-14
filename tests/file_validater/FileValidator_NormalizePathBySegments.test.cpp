#include <gtest/gtest.h>

#include "FileValidator.hpp"
#include "lib/exception/ResponseStatusException.hpp"

TEST(FileValidatorTest, NormalizePathBySegments_BasicCases) {
  EXPECT_EQ(FileValidator::NormalizePathBySegments("/a/b/c"),
            "/a/b/c");
  EXPECT_EQ(FileValidator::NormalizePathBySegments("/a/./b/c"),
            "/a/b/c");
  EXPECT_EQ(FileValidator::NormalizePathBySegments("/a/b/./c/"),
            "/a/b/c/");
}

TEST(FileValidatorTest, NormalizePathBySegments_DotDotCases) {
  EXPECT_EQ(FileValidator::NormalizePathBySegments("/a/b/../c"),
            "/a/c");
  EXPECT_EQ(FileValidator::NormalizePathBySegments("/a/../b/c"),
            "/b/c");
}

TEST(FileValidatorTest, NormalizePathBySegments_EdgeCases) {
  EXPECT_EQ(FileValidator::NormalizePathBySegments("/"),
            "/");
  EXPECT_EQ(FileValidator::NormalizePathBySegments("///"),
            "/");
}

// expect throw for paths that try to escape root
TEST(FileValidatorTest, NormalizePathBySegments_RejectEscapeRoot) {
  EXPECT_THROW(FileValidator::NormalizePathBySegments("/../a/b"), std::runtime_error);
  EXPECT_THROW(FileValidator::NormalizePathBySegments("/a/../../b"), std::runtime_error);
  EXPECT_THROW(FileValidator::NormalizePathBySegments("../a/b"), std::runtime_error);
  EXPECT_THROW(FileValidator::NormalizePathBySegments("a/../../b"), std::runtime_error);
}

// reject empty paths
TEST(FileValidatorTest, NormalizePathBySegments_RejectEmptyPaths) {
  EXPECT_THROW(FileValidator::NormalizePathBySegments(""), std::runtime_error);
}

// reject relative paths
TEST(FileValidatorTest, NormalizePathBySegments_RejectRelativePaths) {
  EXPECT_THROW(FileValidator::NormalizePathBySegments("a/b/c"), std::runtime_error);
  EXPECT_THROW(FileValidator::NormalizePathBySegments("."), std::runtime_error);
  EXPECT_THROW(FileValidator::NormalizePathBySegments("./"), std::runtime_error);
  EXPECT_THROW(FileValidator::NormalizePathBySegments("./a/b"), std::runtime_error);
  EXPECT_THROW(FileValidator::NormalizePathBySegments("a/b/."), std::runtime_error);
  EXPECT_THROW(FileValidator::NormalizePathBySegments("././"), std::runtime_error);
}
