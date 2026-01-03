#include <gtest/gtest.h>
#include <fstream>
#include <sys/stat.h>
#include "lib/utils/file_utils.hpp"
#include "lib/exception/ResponseStatusException.hpp"
#include "lib/http/Status.hpp"

class FileUtilsTest : public ::testing::Test {
protected:
  std::string base = "tests/tmp";

  void SetUp() override {
    mkdir("tests", 0755);
    mkdir(base.c_str(), 0755);
    // readable but non-executable file
    std::ofstream(base + "/readable_non_exec.txt") << "hello";
    chmod((base + "/readable_non_exec.txt").c_str(), 0644);
    // unreadable file
    std::ofstream(base + "/unreadable_non_exec.txt") << "secret";
    chmod((base + "/unreadable_non_exec.txt").c_str(), 0000);
    // executable file
    std::ofstream(base + "/exec.sh") << "#!/bin/sh\necho hi\n";
    chmod((base + "/exec.sh").c_str(), 0755);
    // non-executable file
    std::ofstream(base + "/non_exec.txt") << "no exec";
    chmod((base + "/non_exec.txt").c_str(), 0644);
    // directory
    mkdir((base + "/dir").c_str(), 0755);
    // unwritable parent directory
    // (parent dir must be executable and writable when deleting a file)
    mkdir((base + "/unwritable_dir").c_str(), 0555);
    std::string delete_test_file = base + "/unwritable_dir/delete_test.txt";
    std::ofstream(delete_test_file) << "can't delete me";
    chmod(delete_test_file.c_str(), 0644);
  }

  void TearDown() override {
    chmod((base + "/unreadable_non_exec.txt").c_str(), 0644);
    chmod((base + "/unwritable_dir/delete_test.txt").c_str(), 0644);
    chmod((base + "/unwritable_dir").c_str(), 0755);
    unlink((base + "/readable_non_exec.txt").c_str());
    unlink((base + "/unreadable_non_exec.txt").c_str());
    unlink((base + "/exec.sh").c_str());
    unlink((base + "/non_exec.txt").c_str());
    unlink((base + "/unwritable_dir/delete_test.txt").c_str());
    rmdir((base + "/unwritable_dir").c_str());
    rmdir((base + "/dir").c_str());
    rmdir(base.c_str());
  }
};

// static file GET
// CheckReadableRegularFileOrThrow(const std::string& path)
TEST_F(FileUtilsTest, CheckReadableRegularFileOrThrow_OK) {
  EXPECT_NO_THROW(
    lib::utils::CheckReadableRegularFileOrThrow(base + "/readable_non_exec.txt")
  );
}

TEST_F(FileUtilsTest, CheckReadableRegularFileOrThrow_Unreadable) {
  EXPECT_THROW(
    lib::utils::CheckReadableRegularFileOrThrow(base + "/unreadable_non_exec.txt"),
    lib::exception::ResponseStatusException
  );
}

// CGI script execution (GET/POST)
// CheckExecutableCgiScriptOrThrow(const std::string& path) 
TEST_F(FileUtilsTest, CheckExecutableCgiScriptOrThrow_OK) {
  EXPECT_NO_THROW(
    lib::utils::CheckExecutableCgiScriptOrThrow(base + "/exec.sh")
  );
}

TEST_F(FileUtilsTest, CheckExecutableCgiScriptOrThrow_NotFound) {
  EXPECT_THROW(
    lib::utils::CheckExecutableCgiScriptOrThrow(base + "/nonexistent.sh"),
    lib::exception::ResponseStatusException
  );
}

TEST_F(FileUtilsTest, CheckExecutableCgiScriptOrThrow_NonExecutable) {
  EXPECT_THROW(
    lib::utils::CheckExecutableCgiScriptOrThrow(base + "/non_exec.txt"),
    lib::exception::ResponseStatusException
  );
}

TEST_F(FileUtilsTest, CheckExecutableCgiScriptOrThrow_Unreadable) {
  EXPECT_THROW(
    lib::utils::CheckExecutableCgiScriptOrThrow(base + "/unreadable_non_exec.txt"),
    lib::exception::ResponseStatusException
  );
}

// Tests for CheckDeletableRegularFileOrThrow(const std::string& path)
TEST_F(FileUtilsTest, CheckDeletableRegularFileOrThrow_NotRegular) {
  EXPECT_THROW(
    lib::utils::CheckDeletableRegularFileOrThrow(base + "/dir"),
    lib::exception::ResponseStatusException
  );
}

TEST_F(FileUtilsTest, CheckDeletableRegularFileOrThrow_NoPermission) {
  EXPECT_THROW(
    lib::utils::CheckDeletableRegularFileOrThrow(base + "/unwritable_dir/delete_test.txt"),
    lib::exception::ResponseStatusException
  );
}