#include <gtest/gtest.h>
#include "HttpRequest.hpp"
#include "lib/exception/ResponseStatusException.hpp"
#include "lib/http/Status.hpp"

class HttpRequestMaxBodyTest : public ::testing::Test {
protected:
    void SetUp() override {
        request_.SetMaxBodySizeLimit(100);
    }

    HttpRequest request_;
};

TEST_F(HttpRequestMaxBodyTest, SingleChunkExceedsLimit_Throws413) {
    // limit is 100 bytes, sending a chunk of 2048 bytes
    std::string header =
        "POST /upload HTTP/1.1\r\n"
        "Host: example.com\r\n"
        "Transfer-Encoding: chunked\r\n"
        "\r\n"
        "800\r\n";  // 2048 bytes in hex

    std::string body_ = std::string(2048, 'A') + "\r\n"
        "0\r\n"
        "\r\n";

    try {
        request_.Parse(header.c_str(), header.size());
        request_.Parse(body_.c_str(), body_.size());
        FAIL() << "Expected ResponseStatusException";
    } catch (const lib::exception::ResponseStatusException& e) {
        EXPECT_EQ(lib::http::kPayloadTooLarge, e.GetStatus());
    }
}

TEST_F(HttpRequestMaxBodyTest, MultipleChunksExceedLimit_Throws413) {
    // limit is 100 bytes, sending two chunks of 64 bytes each (total 128 bytes)
    std::string header =
        "POST /upload HTTP/1.1\r\n"
        "Host: example.com\r\n"
        "Transfer-Encoding: chunked\r\n"
        "\r\n";

    std::string chunk_64bytes = "40\r\n" + std::string(64, 'A') + "\r\n";
    std::string body =
        chunk_64bytes +
        chunk_64bytes +
        "0\r\n"
        "\r\n";  // Total 128 bytes

    try {
        request_.Parse(header.c_str(), header.size());
        request_.Parse(body.c_str(), body.size());
        FAIL() << "Expected ResponseStatusException";
    } catch (const lib::exception::ResponseStatusException& e) {
        EXPECT_EQ(lib::http::kPayloadTooLarge, e.GetStatus());
    }
}

// edge case: exactly at limit
TEST_F(HttpRequestMaxBodyTest, ExactLimitChunked_Allows) {
    HttpRequest req;
    req.SetMaxBodySizeLimit(1024);
    std::string header =
        "POST /upload HTTP/1.1\r\n"
        "Host: example.com\r\n"
        "Transfer-Encoding: chunked\r\n"
        "\r\n";
    std::string body =
        "400\r\n" + std::string(1024, 'A') + "\r\n0\r\n\r\n";  // 1024 bytes
    EXPECT_NO_THROW({
        req.Parse(header.c_str(), header.size());
        req.Parse(body.c_str(), body.size());
    });
    EXPECT_TRUE(req.IsDone());
    EXPECT_EQ(req.GetBody().size(), 1024u);
}

TEST_F(HttpRequestMaxBodyTest, ContentLengthExceedsLimit_Throws413) {
    HttpRequest req;
    req.SetMaxBodySizeLimit(1024);
    std::string header =
        "POST /upload HTTP/1.1\r\n"
        "Host: example.com\r\n"
        "Transfer-Encoding: chunked\r\n"
        "\r\n";
    std::string body =
        "401\r\n" + std::string(1025, 'A') + "\r\n0\r\n\r\n";  // 1025 bytes
    try {
        req.Parse(header.c_str(), header.size());
        req.Parse(body.c_str(), body.size());
        FAIL() << "Expected ResponseStatusException";
    } catch (const lib::exception::ResponseStatusException& e) {
        EXPECT_EQ(lib::http::kPayloadTooLarge, e.GetStatus());
    }
}