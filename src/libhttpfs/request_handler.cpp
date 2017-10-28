#include "request_handler.h"
#include "../libhttpc/http_client.h"
#include "filesystem.h"

#include <string>
#include <boost/filesystem.hpp>
#include <asio.hpp>
#include <iostream>
#include <thread>
#include <fstream>
#include <algorithm>

extern std::string root_dir_path;
static int level = 0;

std::string constructServerMessage(const std::string partial_header, const std::string body)
{
    std::ostringstream oss;
    auto tmp_len = partial_header.length() - 2;
    std::string header;
    if (tmp_len > 0 && partial_header.substr(tmp_len) == "\r\n")
        header = partial_header.substr(0, tmp_len);
    else
        header = partial_header;
    oss << header;
    oss << "\r\nConnection: close" << "\r\nServer: httpfs/0.0.1";
    oss << "\r\nDate: " << now();
    if (!body.empty())
    {
        oss << "\r\nContent-Length: " << body.length();
        oss << "\r\n\r\n" << body;
    }
    return oss.str();
}

std::string generateHtmlMessage(const char* msg, const std::string& title, const std::string& header)
{
    std::ostringstream oss;
    using std::endl;

    oss << "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 3.2 Final//EN\">" << endl;
    oss << "<title>" << title << "</title>" << endl;
    oss << "<h1>" << header << "</h1>" << endl;
    oss << "<p>" << msg << "</p>" << endl;
    return oss.str();
}

std::string httpGetMessage(const HttpMessage& client_msg) noexcept
{
    using namespace boost::filesystem;
    path dir_path;
    if (client_msg.resource_path.substr(0, 6) == "/icons")
        dir_path = icons_dir_path;
    else
        dir_path = root_dir_path;
    path file_path(client_msg.resource_path);
    path full_path = dir_path / file_path;

    if (client_msg.resource_path == "/" || is_directory(full_path))
    {
        try
        {
            std::string body = htmlDirList(root_dir_path, client_msg.resource_path);//jsonDirList(root_dir_path, level);
            std::string partial_header = "HTTP/1.1 200 OK\r\nContent-Type: text/html";
            return constructServerMessage(partial_header, body);
        }
        catch(const boost::filesystem::filesystem_error& ex)
        {
            std::string body(generateHtmlMessage(ex.what(), "500 Internal Server Error",
                                                      "Default directory does not exists on the server"));
            std::string partial_header = "HTTP/1.0 500 Internal Server Error\r\n"
                                         "Content-Type: text/html\r\n";
            return constructServerMessage(partial_header, body);
        }
    }
    try
    {        
        std::string path = full_path.c_str();
        std::string body = fileContent(path, !client_msg.is_text_body);
        auto mime_type = getFileMimeType(path);
        if (mime_type.empty())
            mime_type = "text/plain";
        std::string partial_header = "HTTP/1.1 200 OK\r\nContent-Type: " + mime_type;
        return constructServerMessage(partial_header, body);
    }
    catch (const filesystem_error& ex)
    {
        std::string body(generateHtmlMessage(ex.what(), "404 Not Found",
                                                  "File does not exist"));

        std::string partial_header = "HTTP/1.0 404 NOT FOUND\r\n"
                                     "Content-Type: text/html";
        return constructServerMessage(partial_header, body);
    }
    catch (const std::ifstream::failure&)
    {
        std::string what = client_msg.resource_path + " doesn't exist!";
        std::string body(generateHtmlMessage(what.c_str(), "404 Not Found",
                                                  "File does not exist"));

        std::string partial_header = "HTTP/1.0 404 NOT FOUND\r\n"
                                     "Content-Type: text/html";
        return constructServerMessage(partial_header, body);
    }
}

std::string httpPostMessage(const HttpMessage& client_msg) noexcept
{
    using namespace boost::filesystem;
    path dir_path(root_dir_path);
    path file_path(client_msg.resource_path);
    try
    {
        path full_path = dir_path / file_path;
        bool is_exists = exists(full_path);
        if (is_exists && !is_regular_file(full_path))
        {
            std::string partial_header = "HTTP/1.0 400 Bad Request\r\n"
                                         "Content-Type: text/html";
            std::string msg = std::string(full_path.c_str()) + " is not a regular file";
            std::string body = generateHtmlMessage(msg.c_str(), "400 Bad Request", "No regular file");
            return constructServerMessage(partial_header, body);
        }
        create_directories(full_path.parent_path());
        std::ofstream out;
        out.exceptions(std::ofstream::badbit | std::ofstream::failbit);
        if (client_msg.is_text_body)
            out.open(full_path.c_str());
        else
            out.open(full_path.c_str(), std::ofstream::binary);
        out << client_msg.body;
        std::string partial_header = "HTTP/1.1 200 OK\r\nContent-Type: text/html";
        std::string body = generateHtmlMessage("File created/updated successfully!",
                                   "Operation Successfull", "Operation Successfull");
        return constructServerMessage(partial_header, body);
    }
    catch(const filesystem_error& ex)
    {
        std::string body = generateHtmlMessage(ex.what(), "500 Internal Server Error", "File Operation Failed");
        std::string partial_header = "HTTP/1.0 500 Internal Server Error\r\n"
                                     "Content-Type: text/html";
        return constructServerMessage(partial_header, body);
    }
    catch(const std::ofstream::failure& ex)
    {
        std::string body = generateHtmlMessage(ex.what(), "500 Internal Server Error", "Error in File IO Operations");
        std::string partial_header = "HTTP/1.0 500 Internal Server Error\r\n"
                                     "Content-Type: text/html";
        return constructServerMessage(partial_header, body);
    }
}

std::string prepareReplyMessage(const HttpMessage& client_msg) noexcept
{
    switch (client_msg.message_type)
    {
    case HttpMessageType::Get:
        return httpGetMessage(client_msg);
    case HttpMessageType::Post:
        return httpPostMessage(client_msg);
    default:
        std::string body = generateHtmlMessage("Operation is not implemented",
                                               "501 Not Implemented", "501 Not Implemented");
        std::string partial_header = "HTTP/1.0 501 Not Implemented\r\n"
                                     "Content-Type: text/html";
        return constructServerMessage(partial_header, body);
    }
}

std::string now()
{
    const char* fmt = "%a, %d %b %Y %H:%M:%S %Z";
    std::time_t t = std::time(NULL);
    char mbstr[100];
    if (std::strftime(mbstr, sizeof(mbstr), fmt, std::gmtime(&t)))
        return std::string(mbstr);

    auto start = std::chrono::system_clock::now();
    std::time_t time = std::chrono::system_clock::to_time_t(start);
    std::string ret(std::ctime(&time));
    return ret;
}
