 #pragma once

#include <iosfwd>

class HttpMessage;

extern std::string root_dir_path;
extern std::string icons_dir_path;
extern bool verbose;
extern bool no_cache;
extern int children_level;

std::string prepareReplyMessage(const HttpMessage& client_msg) noexcept;
std::string now();
