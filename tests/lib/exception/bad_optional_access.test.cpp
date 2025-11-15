#include "lib/exception/bad_optional_access.hpp"

#include <gtest/gtest.h>

#include "lib/exception/messages.hpp"

TEST(BadOptionalAccessTest, WhatReturnsExpectedString) {
  lib::exception::bad_optional_access e;
  EXPECT_STREQ(e.what(), lib::exception::kBadOptionalAccessMsg);
}
