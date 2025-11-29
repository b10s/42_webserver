#ifndef LIB_TYPE_OPTIONAL_HPP_
#define LIB_TYPE_OPTIONAL_HPP_

#include "lib/exception/BadOptionalAccess.hpp"

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

  T Value() const {
    if (!has_value_) throw lib::exception::BadOptionalAccess();
    return value_;
  }

  T ValueOr(const T& def) const {
    if (has_value_) return value_;
    return def;
  }

  bool HasValue() const {
    return has_value_;
  }

  void Reset() {
    has_value_ = false;
  }
};

}  // namespace type
}  // namespace lib

#endif
