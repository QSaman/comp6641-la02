#include <iostream>
#include <boost/filesystem.hpp>
using namespace boost::filesystem;

int main(int argc, char* argv[])
{
    std::string str = "/media/NixHddData/MyStuff/Star Wars/FanFilms/DARTH MAUL - Apprentice - A Star Wars Fan-Film-Djo_91jN3Pk.webm";
  std::cout << str << " " << file_size(str) << '\n';
  return 0;
}
 
