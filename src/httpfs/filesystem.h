#pragma once

#include <iosfwd>

std::string textDirList(const std::string& dir_path);
std::string jsonDirList(const std::string& dir_path, int level = 0);
std::string xmlDirList(const std::string& dir_path);


