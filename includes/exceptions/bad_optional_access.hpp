#ifndef BAD_OPTIONAL_ACCESS_HPP_
#define BAD_OPTIONAL_ACCESS_HPP_

#include <exception>

class bad_optional_access : public std::exception {
 public:
  const char* what() const throw();
};

#endif
