#include <iostream>
#include <string>
#include <exception>
#include <thread>

#include "../libhttpfs/http_server.h"
#include "../libhttpfs/filesystem.h"
#include "../libhttpc/http_client.h"
#include <boost/filesystem.hpp>
#include <asio.hpp>

extern std::string root_dir_path;

int main()
{
    try
    {
        runUdpServer(7777);
        //std::cout << textDirList("/media/NixHddData/MyStuff/Programming/Projects/C++/Comp6461/LabAssignment02/");
        //std::cout << jsonDirList("/media/NixHddData/MyStuff/test/force_users/", 2);
        std::cout << jsonDirList("/media/NixHddData/MyStuff/test/force_users2/", 2);
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
 
