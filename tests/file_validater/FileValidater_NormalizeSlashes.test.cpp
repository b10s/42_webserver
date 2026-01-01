#include <gtest/gtest.h>

#include "FileValidater.hpp"

TEST(FileValidatorTest, NormalizeSlashes_MultipleSlashes) {
  std::string input = "/path////to///file.txt";
  std::string expected = "/path/to/file.txt";
  std::string output = FileValidator::NormalizeSlashes(input);
  EXPECT_EQ(output, expected);
}

TEST(FileValidatorTest, NormalizeSlashes_LeadingAndTrailingSlashes) {
  std::string input = "////leading/and/trailing////";
  std::string expected = "/leading/and/trailing/";
  std::string output = FileValidator::NormalizeSlashes(input);
  EXPECT_EQ(output, expected);
}

TEST(FileValidatorTest, NormalizeSlashes_NoExtraSlashes) {
  std::string input = "/normal/path/file.txt";
  std::string expected = "/normal/path/file.txt";
  std::string output = FileValidator::NormalizeSlashes(input);
  EXPECT_EQ(output, expected);
}

TEST(FileValidatorTest, NormalizeSlashes_OnlySlashes) {
  std::string input = "//////";
  std::string expected = "/";
  std::string output = FileValidator::NormalizeSlashes(input);
  EXPECT_EQ(output, expected);
}

TEST(FileValidatorTest, NormalizeSlashes_EmptyString) {
  std::string input = "";
  std::string expected = "";
  std::string output = FileValidator::NormalizeSlashes(input);
  EXPECT_EQ(output, expected);
}