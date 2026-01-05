#ifndef LIB_PARSER_STREAMPARSER_HPP_
#define LIB_PARSER_STREAMPARSER_HPP_

#include <string>

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

  StreamParser();
  virtual ~StreamParser();

  void Parse(const char* data, size_t len);

 protected:
  // Derived classes use these.
  std::string buffer_;
  size_t buffer_read_pos_;
  State state_;

  // Derived classes implement these.
  virtual bool AdvanceHeader() = 0;
  virtual bool AdvanceBody() = 0;

  // Error hooks: derived decides which exception/status to throw.
  virtual void OnInternalStateError() = 0;
  virtual void OnExtraDataAfterDone() = 0;

  bool IsCRLF(const char* p) const;
  std::string::size_type FindEndOfHeader(const std::string& payload);

  void BumpLenOrThrow(size_t& total, size_t inc, size_t max_size) const;

  // we allow only single space after ":" and require CRLF at end
  // OWS (optional whitespace) is not supported for simplicity
  const char* ReadHeaderLine(const char* req, std::string& key,
                             std::string& value, size_t& total_len,
                             size_t max_size);
};

}  // namespace parser
}  // namespace lib

#endif  // LIB_PARSER_STREAM_PARSER_HPP_
