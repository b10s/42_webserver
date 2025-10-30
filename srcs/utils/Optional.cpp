#include "utils/Optional.hpp"
#include "exceptions/bad_optional_access.hpp"

// Constructor
template <typename T>
Optional<T>::Optional() : has_value_(false) {}

template <typename T>
Optional<T>::Optional(const T& v) : value_(v), has_value_(true) {}

template <typename T>
Optional<T>::Optional(const Optional& other)
    : value_(other.value_), has_value_(other.has_value_) {}

// Destructor
template <typename T>
Optional<T>::~Optional() {}

// Copy assignment operator overload
template <typename T>
Optional<T>& Optional<T>::operator=(const Optional& rhs) {
  if (this != &rhs) {
    has_value_ = rhs.has_value_;
    value_ = rhs.value_;
  }
  return *this;
}

template <typename T>
T Optional<T>::value() const {
  if (!has_value_) throw bad_optional_access();
  return value_;
}

template <typename T>
T Optional<T>::value_or(const T& def) const {
  if (has_value_) return value_;
  return def;
}

template <typename T>
bool Optional<T>::has_value() const {
  return has_value_;
}

template <typename T>
void Optional<T>::reset() {
  has_value_ = false;
}
