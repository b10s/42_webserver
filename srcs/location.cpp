#include "ConfigParser.hpp"

Location::Location()
    : methods_(),
      name_("/"),
      root_("./"),
      autoindex_(false),
      index_files_(),
      upload_path_(),
      redirect_(),
      cgi_enabled_(false),
      has_allow_methods_(false),
      has_root_(false),
      has_autoindex_(false),
      has_index_directive_(false),
      has_upload_path_(false),
      has_redirect_(false),
      has_cgi_enabled_(false) {
}
