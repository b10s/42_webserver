#ifndef LIB_UTILS_STRING_UTILS_HPP_
#define LIB_UTILS_STRING_UTILS_HPP_

#include <string>

#include "lib/type/Optional.hpp"

namespace lib {
namespace utils {

std::string ToLowerAscii(const std::string& s);
lib::type::Optional<long> StrToLong(const std::string& s);

}  // namespace utils
}  // namespace lib

#endif
