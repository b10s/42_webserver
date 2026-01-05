#include "lib/parser/StreamParser.hpp"

#include <string>

#include "lib/exception/ResponseStatusException.hpp"
#include "lib/http/CharValidation.hpp"
#include "lib/http/Status.hpp"

namespace lib {
namespace parser {

StreamParser::StreamParser() : buffer_read_pos_(0), state_(kHeader) {
}

StreamParser::~StreamParser() {
}

void StreamParser::Parse(const char* data, size_t len) {
  buffer_.append(data, len);
  for (;;) {
    switch (state_) {
      case kHeader:
        if (!AdvanceHeader()) return;
        if (state_ != kBody) OnInternalStateError();
        continue;

      case kBody:
        if (!AdvanceBody()) return;
        if (state_ != kDone) OnInternalStateError();
        return;

      case kDone:
        OnExtraDataAfterDone();
    }
  }
}

bool StreamParser::IsCRLF(const char* p) const {
  return p != NULL && p[0] == '\r' && p[1] == '\n';
}

std::string::size_type StreamParser::FindEndOfHeader(
    const std::string& payload) {
  size_t pos = payload.find("\r\n\r\n");
  if (pos != std::string::npos) return pos + 4;
  pos = payload.find("\n\n");
  if (pos != std::string::npos) return pos + 2;
  return std::string::npos;
}

void StreamParser::BumpLenOrThrow(size_t& total, size_t inc,
                                  size_t max_size) const {
  if (inc > max_size - total) {
    throw lib::exception::ResponseStatusException(
        lib::http::kRequestHeaderFieldsTooLarge);
  }
  total += inc;
}

const char* StreamParser::ReadHeaderLine(const char* req, std::string& key,
                                         std::string& value, size_t& total_len,
                                         size_t max_size) {
  size_t i = 0;
  while (req[i] && req[i] != ':') {
    if (!lib::http::IsValidHeaderChar(req[i])) {
      throw lib::exception::ResponseStatusException(lib::http::kBadRequest);
    }
    BumpLenOrThrow(total_len, 1, max_size);
    ++i;
  }
  if (req[i] == '\0' || req[i] != ':' ||
      req[i + 1] != ' ') {  // must be ": ", not ":" or end of string
    throw lib::exception::ResponseStatusException(lib::http::kBadRequest);
  }
  key.assign(req, i);
  // skip": "
  i += 2;
  BumpLenOrThrow(total_len, 2, max_size);
  req += i;
  size_t vlen = 0;
  while (req[vlen] && req[vlen] != '\r' && req[vlen] != '\n') {
    if (!lib::http::IsValidHeaderChar(req[vlen])) {
      throw lib::exception::ResponseStatusException(lib::http::kBadRequest);
    }
    BumpLenOrThrow(total_len, 1, max_size);
    ++vlen;
  }
  value.assign(req, vlen);  // value can be empty
  if (IsCRLF(req + vlen)) {
    BumpLenOrThrow(total_len, 2, max_size);  // skip CRLF
    return req + vlen + 2;
  } else if (req[vlen] == '\n') {
    BumpLenOrThrow(total_len, 1, max_size);  // skip LF
    return req + vlen + 1;
  }
  throw lib::exception::ResponseStatusException(lib::http::kBadRequest);
}

}  // namespace parser
}  // namespace lib
