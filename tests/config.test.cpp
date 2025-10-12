#include <gtest/gtest.h>

TEST(Test2, BasicAssertions) {
  EXPECT_EQ(1 * 1, 1);
  EXPECT_EQ("hoge", "hoge");
}
