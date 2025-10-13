#include "config_parser.hpp"

Location::Location()
    : methods_(),
      name_("/"),
      root_("./"),
      autoindex_(false),
      indexFiles_(),
      extensions_(),
      uploadPath_(),
      redirect_(),
      cgiPath_()
{}