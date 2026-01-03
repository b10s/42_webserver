#include <gtest/gtest.h>
#include <fstream>
#include <sys/stat.h>
#include <unistd.h>
#include "lib/utils/file_utils.hpp"
#include "lib/exception/ResponseStatusException.hpp"
#include "lib/http/Status.hpp"

class FileUtilsTest : public ::testing::Test {
protected:
  std::string base = "tests/tmp";

  void SetUp() override {
    mkdir("tests", 0755);
    mkdir(base.c_str(), 0755);

    // readable file
    std::ofstream(base + "/readable.txt") << "hello";
    chmod((base + "/readable.txt").c_str(), 0644);

    // unreadable file
    std::ofstream(base + "/unreadable.txt") << "secret";
    chmod((base + "/unreadable.txt").c_str(), 0000);

    // executable file
    std::ofstream(base + "/exec.sh") << "#!/bin/sh\necho hi\n";
    chmod((base + "/exec.sh").c_str(), 0755);

    // non-executable file
    std::ofstream(base + "/non_exec.txt") << "no exec";
    chmod((base + "/non_exec.txt").c_str(), 0644);

    // directory
    mkdir((base + "/dir").c_str(), 0755);
  }

  void TearDown() override {
    chmod((base + "/unreadable.txt").c_str(), 0644);
    unlink((base + "/readable.txt").c_str());
    unlink((base + "/unreadable.txt").c_str());
    unlink((base + "/exec.sh").c_str());
    unlink((base + "/non_exec.txt").c_str());
    rmdir((base + "/dir").c_str());
    rmdir(base.c_str());
  }
};

TEST_F(FileUtilsTest, IsDirectory) {
  EXPECT_TRUE(lib::utils::IsDirectory(base + "/dir"));
  EXPECT_FALSE(lib::utils::IsDirectory(base + "/readable.txt"));
  EXPECT_FALSE(lib::utils::IsDirectory(base + "/nonexistent"));
}

// TEST_F(FileUtilsTest, ReadableFile_OK) {
//   EXPECT_NO_THROW(
//     lib::utils::EnsureReadableRegularFileOrThrow(base + "/readable.txt")
//   );
// }

// TEST_F(FileUtilsTest, ReadableFile_NotFound) {
//   EXPECT_THROW(
//     lib::utils::EnsureReadableRegularFileOrThrow(base + "/missing.txt"),
//     lib::exception::ResponseStatusException
//   );
// }

// TEST_F(FileUtilsTest, ReadableFile_IsDirectory) {
//   EXPECT_THROW(
//     lib::utils::EnsureReadableRegularFileOrThrow(base + "/dir"),
//     lib::exception::ResponseStatusException
//   );
// }

// TEST_F(FileUtilsTest, ReadableFile_NoPermission) {
//   EXPECT_THROW(
//     lib::utils::EnsureReadableRegularFileOrThrow(base + "/unreadable.txt"),
//     lib::exception::ResponseStatusException
//   );
// }

// TEST_F(FileUtilsTest, ExecutableFile_OK) {
//   EXPECT_NO_THROW(
//     lib::utils::EnsureExecutableRegularFileOrThrow(base + "/exec.sh")
//   );
// }

// TEST_F(FileUtilsTest, ExecutableFile_NotExecutable) {
//   EXPECT_THROW(
//     lib::utils::EnsureExecutableRegularFileOrThrow(base + "/non_exec.txt"),
//     lib::exception::ResponseStatusException
//   );
// }


