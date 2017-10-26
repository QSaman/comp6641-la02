#pragma once

#include <iosfwd>

[[noreturn]] void runUdpServer(unsigned short port);
std::string constructServerMessage(const std::string partial_header, const std::string body);
