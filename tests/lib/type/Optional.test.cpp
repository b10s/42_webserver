#include "lib/type/Optional.hpp"

#include <gtest/gtest.h>

#include "lib/exception/BadOptionalAccess.hpp"

TEST(OptionalTest, ValueMethodReturnsValue) {
  lib::type::Optional<int> test1 = 1;
  lib::type::Optional<int> test2(2);

  EXPECT_EQ(test1.Value(), 1);
  EXPECT_EQ(test2.Value(), 2);
}

TEST(OptionalTest, ValueMethodThrowsException) {
  lib::type::Optional<int> test;

  EXPECT_THROW(test.Value(), lib::exception::BadOptionalAccess);
}

TEST(OptionalTest, OperatorOverload) {
  lib::type::Optional<int> test1 = 1;
  lib::type::Optional<int> test2 = test1;

  EXPECT_EQ(test2.Value(), 1);
}

TEST(OptionalTest, ValueOrMethodReturnsValue) {
  lib::type::Optional<int> test = 1;

  EXPECT_EQ(test.ValueOr(-1), 1);
}

TEST(OptionalTest, ValueOrMethodReturnsDefault) {
  lib::type::Optional<int> test;

  EXPECT_EQ(test.ValueOr(-1), -1);
}

TEST(OptionalTest, ReSetMethod) {
  lib::type::Optional<int> test = 1;
  test.Reset();

  EXPECT_THROW(test.Value(), lib::exception::BadOptionalAccess);
}
