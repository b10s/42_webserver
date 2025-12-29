#ifndef LOCATIONMATCH_HPP_
#define LOCATIONMATCH_HPP_

#include <string>

class Location;  // Forward declaration

// holds the result of location matching
// remainder is the relative path after the location prefix
struct LocationMatch {
  const Location* loc;    // pointer to the best matched location
  std::string remainder;  // always starts with '/'
};

/*
LocationMatch m = FindLocationForUri(uri);
filesystem_path = m.loc->GetRoot() + m.remainder;
*/

#endif  // LOCATIONMATCH_HPP_
