#include "HttpRequest.hpp"

/**
 * @brief Main function that incrementally parses an entire HTTP request.
 *
 * @param payload Newly received data (null-terminated string)
 *
 * @throw lib::exception::ResponseStatusException
 *        - BAD_REQUEST: malformed request
 *        - INTERNAL_SERVER_ERROR: unexpected error during parsing
 */
void HttpRequest::ParseRequest(const char* data, size_t len) {
  try {
    buffer_.append(data, len);

    for (;;) {
      switch (progress_) {
        case kHeader:
          if (!AdvanceHeaderParsing()) return;
          if (progress_ != kBody) {
            throw lib::exception::ResponseStatusException(
                lib::http::kInternalServerError);
          }
          continue;
        case kBody:
          if (!AdvanceBodyParsing()) return;
          if (progress_ != kDone) {
            throw lib::exception::ResponseStatusException(
                lib::http::kInternalServerError);
          }
          return;
        case kDone:
          throw lib::exception::ResponseStatusException(
              lib::http::kBadRequest);  // extra data after done
      }
    }
  } catch (lib::exception::ResponseStatusException& e) {
    throw;
  } catch (std::exception&) {
    throw lib::exception::ResponseStatusException(
        lib::http::kInternalServerError);
  }
}
