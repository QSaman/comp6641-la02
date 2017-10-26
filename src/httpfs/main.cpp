#include <iostream>
#include <string>
#include <exception>
#include <thread>

#include "filesystem.h"
#include "../libhttpc/http_client.h"
#include <boost/filesystem.hpp>
#include <asio.hpp>

static std::string root_dir_path = "./resources/https_root";
static int level = 0;

std::string generateHtmlErrorMessage(const char* msg, const std::string& title, const std::string& header)
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
    std::string reply_msg;
    if (client_msg.resource_path == "/")
    {
        try
        {
            std::string body = jsonDirList(root_dir_path, level);
            std::string partial_header = "HTTP/1.1 200 OK\r\nContent-Type: application/json";
            return HttpClient::constructServerMessage(partial_header, body);
        }
        catch(const boost::filesystem::filesystem_error& ex)
        {
            std::string body(generateHtmlErrorMessage(ex.what(), "404 Not Found",
                                                      "Default directory does not exists on the server"));
            std::string partial_header = "HTTP/1.0 404 NOT FOUND\r\n"
                                         "Content-Type: text/html\r\n";
            return HttpClient::constructServerMessage(partial_header, body);
        }
    }
    using namespace boost::filesystem;
    path dir_path(root_dir_path);
    path file_path(client_msg.resource_path);
    try
    {
        std::string path = (dir_path / file_path).c_str();
        std::string body = fileContent(path);
        std::string partial_header = "HTTP/1.1 200 OK\r\nContent-Type: text/plain";
        return HttpClient::constructServerMessage(partial_header, body);
    }
    catch (const boost::filesystem::filesystem_error& ex)
    {
        std::string body(generateHtmlErrorMessage(ex.what(), "404 Not Found",
                                                  "File does not exist"));

        std::string partial_header = "HTTP/1.0 404 NOT FOUND\r\n"
                                     "Content-Type: text/html\r\n";
        return HttpClient::constructServerMessage(partial_header, body);
    }
    return reply_msg;
}

std::string prepareReplyMessage(const HttpMessage& client_msg) noexcept
{
    std::string reply_msg;
    switch (client_msg.message_type)
    {
    case HttpMessageType::Get:
        return httpGetMessage(client_msg);
    case HttpMessageType::Post:
        break;
    default:
        //TODO
        break;
    }

    return reply_msg;
}

void handleClientHttpRequest(asio::ip::tcp::socket active_socket)
{    
    asio::error_code ec;
    std::string http_header;

    asio::streambuf header_sb;
    auto n = asio::read_until(active_socket, header_sb, "\r\n\r\n", ec);
    if (ec && ec != asio::error::eof)
    {
        std::cerr << "Error in reading data (" << ec.value() << "): " << ec.message() << std::endl;
        exit(ec.value());
    }
    auto bufs = header_sb.data();
    http_header = std::string(asio::buffers_begin(bufs), asio::buffers_begin(bufs) + n);
    std::cout << "header:" << std::endl << http_header << std::endl;
    HttpMessage http_message = HttpClient::extractMessage(http_header, false);
    std::string length_str = http_message.http_header["Content-Length"];
    std::istringstream iss(length_str);
    unsigned int length;
    iss >> length;
    asio::streambuf body_sb;
    n = asio::read(active_socket, body_sb, asio::transfer_exactly(length), ec);
    if (ec || n != length)
    {
        std::cerr << "Error in reading data (" << ec.value() << "): " << ec.message() << std::endl;
        exit(ec.value());
    }
    bufs = body_sb.data();
    auto body = std::string(asio::buffers_begin(bufs), asio::buffers_begin(bufs) + n);
    http_message.body = body;
    std::cout << "body: " << std::endl << body << std::endl;
    asio::write(active_socket, asio::buffer(prepareReplyMessage(http_message)), ec);
    if (ec)
        std::cerr << "Error in writing data (" << ec.value() << "): " << ec.message() << std::endl;
}

[[noreturn]] void runUdpServer(unsigned short port)
{
    using asio::ip::tcp;
    asio::io_service io_service;
    tcp::acceptor passive_socket(io_service, tcp::endpoint(tcp::v4(), port));
    while (true)
    {
        tcp::socket active_socket(io_service);
        passive_socket.accept(active_socket);
        std::thread(handleClientHttpRequest, std::move(active_socket)).detach();
    }
}


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
 
