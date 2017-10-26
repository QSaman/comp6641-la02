#pragma once

#include <iosfwd>

extern std::string root_dir_path;

void runUdpServer(unsigned short port);
std::string constructServerMessage(const std::string partial_header, const std::string body);
