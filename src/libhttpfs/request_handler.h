 #pragma once

#include <iosfwd>

class HttpMessage;

extern std::string root_dir_path;

std::string prepareReplyMessage(const HttpMessage& client_msg) noexcept;
