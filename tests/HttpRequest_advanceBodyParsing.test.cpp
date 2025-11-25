#include <gtest/gtest.h>

#include <stdexcept>
#include <string>

#include "HttpRequest.hpp"
#include "enums.hpp"

class HttpRequestParseUri : public ::testing::Test {
 protected:
  HttpRequest req;
};

// =============== Happy path ===============
