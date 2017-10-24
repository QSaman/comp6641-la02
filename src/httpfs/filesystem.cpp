#include "filesystem.h"

#include <string>
#include <iostream>

#include <boost/filesystem.hpp>
#include <nlohmann/json.hpp>

boost::filesystem::path createBoostDirPath(const std::string& dir_path)
{
    using namespace boost::filesystem;
    path p(dir_path);
    if (!exists(p))
    {
        //TODO
    }
    if (!is_directory(p))
    {
        //TODO
    }
    return p;
}

std::string textDirList(const std::string& dir_path)
{    
    using namespace boost::filesystem;
    std::string ret;
    path p = createBoostDirPath(dir_path);
    for (directory_entry& f : directory_iterator(p))
        ret += std::string(f.path().filename().c_str()) + "\n";
    return ret;
}

nlohmann::json jsonDirList(boost::filesystem::path p, int level)
{
    using namespace boost::filesystem;
    using json = nlohmann::json;

    nlohmann::json ret;
    for (directory_entry& f : directory_iterator(p))
    {
        bool directory = false;
        nlohmann::json object = json::object();
        object["name"] = f.path().filename().c_str();
        if (is_directory(f))
        {
            object["type"] = "directory";
            directory = true;
        }
        else if (is_regular_file(f))
            object["type"] = "file";
        else
            object["type"] = "unknown";
        if (directory && level > 0)
            object["children"] = jsonDirList(f, level - 1);
        ret.push_back(object);
    }
    return ret;
}

std::string jsonDirList(const std::string& dir_path, int level)
{
    using namespace boost::filesystem;
    std::string ret;
    path p = createBoostDirPath(dir_path);

    nlohmann::json jres = jsonDirList(p, level);
    ret = jres.dump(2);
    return ret;
}

std::string xmlDirList(const std::string& dir_path)
{
    return "";
}
