#ifndef OPTIONAL_HPP_
#define OPTIONAL_HPP_

template <typename T>
class Optional {
 private:
  T value_;
  bool has_value_;

 public:
  // Constructor
  Optional();
  Optional(const T&);
  Optional(const Optional&);

  // Destructor
  ~Optional();

  // Copy assignment operator overload
  Optional& operator=(const Optional&);

  T value() const;
  T value_or(const T&) const;
  bool has_value() const;
  void reset();
};

#endif
