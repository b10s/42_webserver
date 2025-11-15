#include "lib/exception/bad_optional_access.hpp"

#include <gtest/gtest.h>

#include "lib/exception/messages.hpp"

TEST(BadOptionalAccessTest, WhatReturnsExpectedString) {
  lib::exception::BadOptionalAccess e;
  EXPECT_STREQ(e.what(), lib::exception::kBadOptionalAccessMsg);
}
