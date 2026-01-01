#include <gtest/gtest.h>

#include "FileValidater.hpp"

TEST(FileValidatorTest,IsPathUnderDocumentRoot_UnderDocumentRoot) {
  std::string document_root = "/var/www/html";
  std::string path = "/var/www/html/images/pic.jpg";
  EXPECT_TRUE(FileValidator::IsPathUnderDocumentRoot(path, document_root));
}

TEST(FileValidatorTest,IsPathUnderDocumentRoot_AtDocumentRoot) {
  std::string document_root = "/var/www/html";
  std::string path = "/var/www/html";
  EXPECT_TRUE(FileValidator::IsPathUnderDocumentRoot(path, document_root));
}

TEST(FileValidatorTest,IsPathUnderDocumentRoot_OutsideDocumentRoot) {
  std::string document_root = "/var/www/html";
  std::string path = "/var/www/other/file.txt";
  EXPECT_FALSE(FileValidator::IsPathUnderDocumentRoot(path, document_root));
}

TEST(FileValidatorTest,IsPathUnderDocumentRoot_SimilarPrefix) {
  std::string document_root = "/var/www/html";
  std::string path = "/var/www/html_extra/file.txt";
  EXPECT_FALSE(FileValidator::IsPathUnderDocumentRoot(path, document_root));
}

TEST(FileValidatorTest,IsPathUnderDocumentRoot_RootDirectory) {
  std::string document_root = "/";
  std::string path = "/etc/passwd";
  EXPECT_TRUE(FileValidator::IsPathUnderDocumentRoot(path, document_root));
}

TEST(FileValidatorTest,IsPathUnderDocumentRoot_EmptyDocumentRoot) {
  std::string document_root = "";
  std::string path = "/some/path/file.txt";
  EXPECT_FALSE(FileValidator::IsPathUnderDocumentRoot(path, document_root));
}