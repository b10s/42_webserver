#include <gtest/gtest.h>

#include "FileValidator.hpp"

TEST(FileValidatorTest, SplitPathSegments_BasicCases) {
  std::vector<std::string> expected1 = {"a", "b", "c"};
  EXPECT_EQ(FileValidator::SplitPathSegments("/a/b/c"), expected1);

  std::vector<std::string> expected2 = {"path", "to", "file.txt"};
  EXPECT_EQ(FileValidator::SplitPathSegments("/path/to/file.txt"), expected2);

  std::vector<std::string> expected3 = {"leading", "and", "trailing"};
  EXPECT_EQ(FileValidator::SplitPathSegments("/leading/and/trailing/"), expected3);
}

TEST(FileValidatorTest, SplitPathSegments_EdgeCases) {
  std::vector<std::string> expected1 = {};
  EXPECT_EQ(FileValidator::SplitPathSegments("/"), expected1);

  std::vector<std::string> expected2 = {"single_segment"};
  EXPECT_EQ(FileValidator::SplitPathSegments("/single_segment"), expected2);

  std::vector<std::string> expected3 = {};
  EXPECT_EQ(FileValidator::SplitPathSegments("///"), expected3);

  std::vector<std::string> expected4 = {};
  EXPECT_EQ(FileValidator::SplitPathSegments(""), expected4);
}
