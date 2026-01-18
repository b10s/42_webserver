#include "HttpRequest.hpp"

/**
 * @brief Wait until the header terminator ("\r\n\r\n") appears, then parse
 * the request line (method, URI, version) and each header field in order.
 *
 * @return
 *   - true if header parsing is complete (progress is set to BODY),
 *   - false if more data is needed.
 *
 * @throw lib::exception::ResponseStatusException
 *        - BAD_REQUEST: malformed request
 *        - INTERNAL_SERVER_ERROR: unexpected exception
 *
 * @post
 *  - consumed data is removed from buffer_
 *  - progress is updated to BODY when header parsing completes
 */

bool HttpRequest::AdvanceHeader() {
  std::string::size_type end_of_header = FindEndOfHeader(buffer_);
  if (end_of_header == std::string::npos) {
    return false;  // need more data
  }
  try {
    const char* begin = buffer_.c_str();
    const char* cur = begin;
    cur = this->ConsumeMethod(cur);
    cur = this->ConsumeUri(cur);
    cur = this->ConsumeVersion(cur);
    cur =
        this->ConsumeHeader(cur);  // throw 413 if content_length_ exceeds limit
    (void)cur;                     // suppress unused variable warning
  } catch (lib::exception::ResponseStatusException&) {
    throw;
  } catch (std::exception&) {
    throw lib::exception::ResponseStatusException(
        lib::http::kInternalServerError);
  }
  buffer_.erase(0, end_of_header);
  state_ = kBody;
  return true;
}
