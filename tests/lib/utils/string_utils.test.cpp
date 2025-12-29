#include "lib/utils/string_utils.hpp"

#include <gtest/gtest.h>

#include <string>

#include "lib/type/Optional.hpp"

TEST(StringUtilsTest, ToLowerAscii) {
  EXPECT_EQ(lib::utils::ToLowerAscii("Hello"), "hello");
  EXPECT_EQ(lib::utils::ToLowerAscii("HELLO"), "hello");
  EXPECT_EQ(lib::utils::ToLowerAscii("hello"), "hello");
  EXPECT_EQ(lib::utils::ToLowerAscii("123!@#"), "123!@#");
  EXPECT_EQ(lib::utils::ToLowerAscii(""), "");
}

TEST(StringUtilsTest, StrToLong_Valid) {
  lib::type::Optional<long> res = lib::utils::StrToLong("123");
  EXPECT_TRUE(res.HasValue());
  EXPECT_EQ(res.Value(), 123);

  res = lib::utils::StrToLong("-456");
  EXPECT_TRUE(res.HasValue());
  EXPECT_EQ(res.Value(), -456);

  res = lib::utils::StrToLong("0");
  EXPECT_TRUE(res.HasValue());
  EXPECT_EQ(res.Value(), 0);
}

TEST(StringUtilsTest, StrToLong_Invalid) {
  EXPECT_FALSE(lib::utils::StrToLong("abc").HasValue());
  EXPECT_FALSE(lib::utils::StrToLong("12a").HasValue());
  EXPECT_FALSE(lib::utils::StrToLong("").HasValue());
  EXPECT_FALSE(lib::utils::StrToLong("   ").HasValue());
  EXPECT_FALSE(lib::utils::StrToLong("12 34").HasValue());
}

TEST(StringUtilsTest, StrToUnsignedShort_Valid) {
  lib::type::Optional<unsigned short> res = lib::utils::StrToUnsignedShort("123");
  EXPECT_TRUE(res.HasValue());
  EXPECT_EQ(res.Value(), 123);

  res = lib::utils::StrToUnsignedShort("0");
  EXPECT_TRUE(res.HasValue());
  EXPECT_EQ(res.Value(), 0);
}

TEST(StringUtilsTest, StrToUnsignedShort_Invalid) {
  EXPECT_FALSE(lib::utils::StrToUnsignedShort("abc").HasValue());
  EXPECT_FALSE(lib::utils::StrToUnsignedShort("12a").HasValue());
  EXPECT_FALSE(lib::utils::StrToUnsignedShort("").HasValue());
  EXPECT_FALSE(lib::utils::StrToUnsignedShort("   ").HasValue());
  EXPECT_FALSE(lib::utils::StrToUnsignedShort("12 34").HasValue());
}

TEST(StringUtilsTest, ToString) {
  EXPECT_EQ(lib::utils::ToString("Hello"), "Hello");
  EXPECT_EQ(lib::utils::ToString(123), "123");
  EXPECT_EQ(lib::utils::ToString(-123), "-123");
  EXPECT_EQ(lib::utils::ToString(123.456), "123.456");
}

TEST(StringUtilsTest, GetFirstToken) {
  EXPECT_EQ(lib::utils::GetFirstToken(lib::type::Optional<std::string>("Basic hogehoge"), " ").Value(), "Basic");
  EXPECT_EQ(lib::utils::GetFirstToken(lib::type::Optional<std::string>("Basic    hogehoge"), " ").Value(), "Basic");
  EXPECT_FALSE(lib::utils::GetFirstToken(lib::type::Optional<std::string>("hogehoge"), " ").HasValue());
}
