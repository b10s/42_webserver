#include <gtest/gtest.h>

#include "FileValidater.hpp"

TEST(FileValidaterTest, SplitPathSegments_BasicCases) {
  std::vector<std::string> expected1 = {"a", "b", "c"};
  EXPECT_EQ(FileValidater::SplitPathSegments("/a/b/c"), expected1);

  std::vector<std::string> expected2 = {"path", "to", "file.txt"};
  EXPECT_EQ(FileValidater::SplitPathSegments("/path/to/file.txt"), expected2);

  std::vector<std::string> expected3 = {"leading", "and", "trailing"};
  EXPECT_EQ(FileValidater::SplitPathSegments("/leading/and/trailing/"), expected3);
}

TEST(FileValidaterTest, SplitPathSegments_EdgeCases) {
  std::vector<std::string> expected1 = {};
  EXPECT_EQ(FileValidater::SplitPathSegments("/"), expected1);

  std::vector<std::string> expected2 = {"single_segment"};
  EXPECT_EQ(FileValidater::SplitPathSegments("/single_segment"), expected2);

  std::vector<std::string> expected3 = {};
  EXPECT_EQ(FileValidater::SplitPathSegments("///"), expected3);

  std::vector<std::string> expected4 = {};
  EXPECT_EQ(FileValidater::SplitPathSegments(""), expected4);
}