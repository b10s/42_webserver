#ifndef LIB_TYPE_OPTIONAL_HPP_
#define LIB_TYPE_OPTIONAL_HPP_

#include "lib/exception/bad_optional_access.hpp"

namespace lib {
namespace type {
template <typename T>
class Optional {
 private:
  T value_;
  bool has_value_;

 public:
  // Constructor
  Optional() : has_value_(false) {
  }

  Optional(const T& v) : value_(v), has_value_(true) {
  }

  Optional(const Optional& other)
      : value_(other.value_), has_value_(other.has_value_) {
  }

  // Destructor
  ~Optional() {
  }

  // Copy assignment operator overload
  Optional<T>& operator=(const Optional& rhs) {
    if (this != &rhs) {
      has_value_ = rhs.has_value_;
      value_ = rhs.value_;
    }
    return *this;
  }

  T value() const {
    if (!has_value_) throw lib::exception::bad_optional_access();
    return value_;
  }

  T value_or(const T& def) const {
    if (has_value_) return value_;
    return def;
  }

  bool has_value() const {
    return has_value_;
  }

  void reset() {
    has_value_ = false;
  }
};

}  // namespace type
}  // namespace lib

#endif
