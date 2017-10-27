 #pragma once

#include <iosfwd>

class HttpMessage;

extern std::string root_dir_path;
extern bool verbose;

std::string prepareReplyMessage(const HttpMessage& client_msg) noexcept;
std::string now();
