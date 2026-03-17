#include "HttpRequest.hpp"
#include <iostream>

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
    std::string header_str = buffer_.substr(0, end_of_header);
    const char* begin = header_str.c_str();
    const char* cur = begin;
    std::cerr << "[DEBUG] before ConsumeMethod" << std::endl;
    cur = this->ConsumeMethod(cur);
    std::cerr << "[DEBUG] after ConsumeMethod" << std::endl;
    cur = this->ConsumeUri(cur);
    std::cerr << "[DEBUG] after ConsumeUri" << std::endl;
    cur = this->ConsumeVersion(cur);
    std::cerr << "[DEBUG] after ConsumeVersion" << std::endl;
    this->ConsumeHeader(cur);  // throw 413 if content_length_ exceeds limit
    std::cerr << "[DEBUG] found end of headers at pos=" << end_of_header << std::endl;
    std::cerr << "[DEBUG] total buffer size=" << buffer_.size() << std::endl;
    std::cerr << "[DEBUG] remaining after headers="
          << (buffer_.size() - end_of_header) << std::endl;
  } catch (lib::exception::ResponseStatusException& e) {
    std::cerr << "[DEBUG] AdvanceHeader caught status=" << e.GetStatus() << std::endl;
    throw;
  } catch (std::exception&) {
    throw lib::exception::ResponseStatusException(
        lib::http::kInternalServerError);
  }
  buffer_.erase(0, end_of_header);
  state_ = kBody;
  return true;
}
