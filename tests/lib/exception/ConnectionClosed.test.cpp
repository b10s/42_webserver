#include "lib/exception/ConnectionClosed.hpp"

#include <gtest/gtest.h>

TEST(ConnectionClosedTest, WhatReturnsExpectedString) {
  lib::exception::ConnectionClosed e;
  EXPECT_STREQ(e.what(), "Connection Closed");
}
