#include <gtest/gtest.h>

#include "FileValidator.hpp"
#include "lib/exception/ResponseStatusException.hpp"

TEST(FileValidaterTest, ValidateAndNormalizePath_BasicCases) {
  // Valid path without unsafe chars or dot-dot segments
  std::string path1 = "/valid/path/to/resource";
  std::string document_root1 = "/valid/path";
  EXPECT_NO_THROW(FileValidator::ValidateAndNormalizePath(path1, document_root1));
  EXPECT_EQ(FileValidator::ValidateAndNormalizePath(path1, document_root1), path1);
}

TEST(FileValidaterTest, ValidateAndNormalizePath_WithDotDotSegments) {
  // Path containing .. segments should throw an exception
  std::string path2 = "/valid/path/../to/resource";
  std::string document_root2 = "/valid/path";
  EXPECT_THROW(FileValidator::ValidateAndNormalizePath(path2, document_root2), lib::exception::ResponseStatusException);
}

TEST(FileValidaterTest, ValidateAndNormalizePath_WithUnsafeChars) {
  // Path containing unsafe characters should throw an exception
  std::string path3 = "/valid/path/to/re\vurce";
  std::string document_root3 = "/valid/path";
  EXPECT_THROW(FileValidator::ValidateAndNormalizePath(path3, document_root3), lib::exception::ResponseStatusException);
}

TEST(FileValidaterTest, ValidateAndNormalizePath_NotUnderDocumentRoot) {
  // Path that resolves outside the document root should throw an exception
  std::string path4 = "/valid/path/to/resource";
  std::string document_root4 = "/valid2/path";
  EXPECT_THROW(FileValidator::ValidateAndNormalizePath(path4, document_root4), lib::exception::ResponseStatusException);
}

TEST(FileValidaterTest, ValidateAndNormalizePath_Normalization) {
  // Path with redundant slashes and single dot segments
  std::string path5 = "/valid//path/./to///resource";
  std::string document_root5 = "/valid/path";
  std::string expected_normalized_path5 = "/valid/path/to/resource";
  EXPECT_NO_THROW(FileValidator::ValidateAndNormalizePath(path5, document_root5));
  EXPECT_EQ(FileValidator::ValidateAndNormalizePath(path5, document_root5), expected_normalized_path5);
}
