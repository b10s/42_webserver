#ifndef LOCATION_HPP_
#define LOCATION_HPP_

class Location {
 private:
  // parsed
  short methods_;
  std::string name_;
  std::string root_;
  bool autoindex_;
  std::vector<std::string> indexFiles_;
  std::vector<std::string> extensions_;
  std::string upoadPath_;
  std::string redirect_;
  std::string cgiPath_;

 public:
  Location();
};

#endif  // LOCATION_HPP_