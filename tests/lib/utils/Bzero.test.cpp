#include "lib/utils/Bzero.hpp"

#include <gtest/gtest.h>

#include <vector>
#include <cstring>


TEST(BzeroTest, ClearsCharArray) {
    char buffer[] = "Hello World";
    const size_t len = sizeof(buffer);
    lib::utils::Bzero(buffer, len);
    for (size_t i = 0; i < len; ++i) {
        EXPECT_EQ(buffer[i], '\0');
    }
}

TEST(BzeroTest, ClearsIntegerArray) {
    int numbers[] = {12345, 67890, -1, 42};
    const size_t size = sizeof(numbers);
    lib::utils::Bzero(numbers, size);
    EXPECT_EQ(numbers[0], 0);
    EXPECT_EQ(numbers[1], 0);
    EXPECT_EQ(numbers[2], 0);
    EXPECT_EQ(numbers[3], 0);
}


TEST(BzeroTest, ClearsOnlyRequestedSize) {
    unsigned char buffer[] = {0xFF, 0xFF, 0xFF, 0xFF};
    
    // delete first 2 bytes
    lib::utils::Bzero(buffer, 2);

    EXPECT_EQ(buffer[0], 0x00);
    EXPECT_EQ(buffer[1], 0x00);
    EXPECT_EQ(buffer[2], 0xFF);
    EXPECT_EQ(buffer[3], 0xFF);
}

TEST(BzeroTest, DoesNothingWhenSizeIsZero) {
    char buffer[] = "KeepMe";
    
    lib::utils::Bzero(buffer, 0);
    EXPECT_STREQ(buffer, "KeepMe");
}
