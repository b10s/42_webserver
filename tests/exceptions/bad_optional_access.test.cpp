#include <gtest/gtest.h>
#include "exceptions/bad_optional_access.hpp"

TEST(BadOptionalAccessTest, WhatReturnsExpectedString) {
    bad_optional_access e;
    EXPECT_STREQ(e.what(), "bad optional access");
}
