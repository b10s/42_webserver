#ifndef LIB_PARSER_STREAMPARSER_HPP_
#define LIB_PARSER_STREAMPARSER_HPP_

#include <map>
#include <sstream>
#include <string>

#include "lib/type/Optional.hpp"

namespace lib {
namespace parser {

// Generic incremental state machine for stream parsing.
// Derived classes implement state-specific advancement.
// Contract:
// - Append input to buffer_
// - Advance(state) must either:
//   - return false if it needs more data (no state change)
//   - or consume from buffer_ and update state_
// - When state becomes kDone, Parse() returns.
class StreamParser {
 public:
  enum State { kHeader = 0, kBody = 1, kDone = 2 };

  StreamParser() : state_(kHeader) {
  }

  virtual ~StreamParser() {
  }

  void Parse(const char* data, size_t len) {
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

 protected:
  // Derived classes use these.
  std::string buffer_;
  State state_;

  // Derived classes implement these.
  virtual bool AdvanceHeader() = 0;
  virtual bool AdvanceBody() = 0;

  // Error hooks: derived decides which exception/status to throw.
  virtual void OnInternalStateError() = 0;
  virtual void OnExtraDataAfterDone() = 0;
};

}  // namespace parser
}  // namespace lib

#endif  // LIB_PARSER_STREAM_PARSER_HPP_
