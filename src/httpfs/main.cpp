#include <iostream>
#include <string>
#include <exception>
#include <thread>
#include <cxxopts.hpp>

#include "../libhttpfs/http_server.h"
#include "../libhttpfs/filesystem.h"
#include "../libhttpc/http_client.h"
#include <boost/filesystem.hpp>
#include <asio.hpp>

extern std::string root_dir_path;
extern std::string mime_file;
extern std::string icons_dir_path;
extern bool no_cache;

std::string root_dir_path = "./resources/httpfs_root";
std::string icons_dir_path = "./resources";
std::string mime_file = "./resources/mime.types";
static cxxopts::Options options("httpfs", "httpfs is a simple file server.");
bool verbose = false;
bool no_cache = false;
static unsigned short port = 8080;

[[noreturn]] void printHelp()
{
    std::cout << options.help({""}) << std::endl;
    std::exit(0);
}

void cli(int argc, char* argv[])
{
    options.add_options()
            ("v,verbose", " Prints debugging messages")
            ("p,port", " Specifies the port number that the server will listen and serve at."
                       " Default is 8080", cxxopts::value<int>(), "num")
            ("d,dir", " Specifies the directory that the server will use to read/write requested"
                      " files. Default is resources/https_root in the current directory when"
                      " launching the application", cxxopts::value<std::string>(), "dir_path")
            ("h,help", "Show this page")
            ("m,mime-file", "A file containing the mapping between file extension and MIME types"
                            " in Apache format. The default is resources/mime.types",
             cxxopts::value<std::string>(), "mime_file")
            ("c,no-cache", "Tell the client to not cache any data. It's useful for debugging purposes (e.g. testing concurrency).");
    options.parse(argc, argv);

    if (options.count("help"))
        printHelp();
    if (options.count("verbose"))
        verbose = true;
    if (options.count("port"))
        port = options["port"].as<unsigned short>();
    if (options.count("dir"))
        root_dir_path = options["dir"].as<std::string>();
    if (options.count("mime-file"))
        mime_file = options["mime-file"].as<std::string>();
    if (options.count("no-cache"))
        no_cache = true;
}

int main(int argc, char* argv[])
{
    cli(argc, argv);
//    try
//    {
        runUdpServer(port);
//        //std::cout << textDirList("/media/NixHddData/MyStuff/Programming/Projects/C++/Comp6461/LabAssignment02/");
//        //std::cout << jsonDirList("/media/NixHddData/MyStuff/test/force_users/", 2);
//        std::cout << jsonDirList("/media/NixHddData/MyStuff/test/force_users2/", 2);
//    }
//    catch (const boost::filesystem::filesystem_error& ex)
//    {
//        std::cout << ex.what() << std::endl;
//    }
//    catch(std::exception ex)
//    {
//        std::cout << ex.what() << std::endl;
//    }
}
 
