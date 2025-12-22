#include "lib/utils/Bzero.hpp"

namespace lib {
namespace utils {

void Bzero(void *s, std::size_t n) {
  unsigned char *s1;

  s1 = static_cast<unsigned char *>(s);
  while (n > 0) {
    *s1 = '\0';
    s1++;
    n--;
  }
}
}  // namespace utils
}  // namespace lib
