#include "lib/type/Optional.hpp"

#include <gtest/gtest.h>

#include "lib/exception/bad_optional_access.hpp"

TEST(OptionalTest, ValueMethodReturnsValue) {
  lib::type::Optional<int> test1 = 1;
  lib::type::Optional<int> test2(2);

  EXPECT_EQ(test1.value(), 1);
  EXPECT_EQ(test2.value(), 2);
}

TEST(OptionalTest, ValueMethodThrowsException) {
  lib::type::Optional<int> test;

  EXPECT_THROW(test.value(), lib::exception::BadOptionalAccess);
}

TEST(OptionalTest, OperatorOverload) {
  lib::type::Optional<int> test1 = 1;
  lib::type::Optional<int> test2 = test1;

  EXPECT_EQ(test2.value(), 1);
}

TEST(OptionalTest, ValueOrMethodReturnsValue) {
  lib::type::Optional<int> test = 1;

  EXPECT_EQ(test.value_or(-1), 1);
}

TEST(OptionalTest, ValueOrMethodReturnsDefault) {
  lib::type::Optional<int> test;

  EXPECT_EQ(test.value_or(-1), -1);
}

TEST(OptionalTest, ResetMethod) {
  lib::type::Optional<int> test = 1;
  test.reset();

  EXPECT_THROW(test.value(), lib::exception::BadOptionalAccess);
}
