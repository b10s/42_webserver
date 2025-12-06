#include "lib/exception/InvalidHeader.hpp"

#include <gtest/gtest.h>

#include "lib/exception/messages.hpp"

TEST(InvalidHeaderTest, WhatReturnsExpectedString) {
  lib::exception::InvalidHeader e;
  EXPECT_STREQ(e.what(), lib::exception::k_invalid_header_msg);
}
