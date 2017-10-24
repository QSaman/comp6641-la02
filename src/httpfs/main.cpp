#include <iostream>
#include <string>
#include <exception>

#include "filesystem.h"
#include <boost/filesystem.hpp>


int main()
{
    try
    {
        //std::cout << textDirList("/media/NixHddData/MyStuff/Programming/Projects/C++/Comp6461/LabAssignment02/");
        std::cout << jsonDirList("/media/NixHddData/MyStuff/test/force_users/", 2);
    }
    catch (const boost::filesystem::filesystem_error& ex)
    {
        std::cout << ex.what() << std::endl;
    }
    catch(std::exception ex)
    {
        std::cout << ex.what() << std::endl;
    }
}
 
