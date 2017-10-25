#include <iostream>
#include <string>
#include <exception>
#include <thread>

#include "filesystem.h"
#include "../libhttpc/http_client.h"
#include <boost/filesystem.hpp>
#include <asio.hpp>

void handleClientHttpRequest(asio::ip::tcp::socket active_socket)
{    
    asio::error_code ec;
    std::string http_header;
    while (true)
    {
        asio::streambuf sb;
        auto n = asio::read_until(active_socket, sb, "\r\n", ec);
        if (ec == asio::error::eof)
            break;
        if (ec)
        {
            std::cerr << "Error in reading data (" << ec.value() << "): " << ec.message() << std::endl;
            exit(ec.value());
        }
        auto bufs = sb.data();
        auto line = std::string(asio::buffers_begin(bufs), asio::buffers_begin(bufs) + n);
        std::cout << line << std::endl;
        if (line == "\r\n")
            break;
        http_header += line;
    }
    HttpMessage http_message = HttpClient::extractMessage(http_header, false);
    std::string length_str = http_message.http_header["Content-Length"];
    std::istringstream iss(length_str);
    unsigned int length;
    iss >> length;
    asio::streambuf sb;
    auto n = asio::read(active_socket, sb, asio::transfer_exactly(length), ec);
    if (ec || n != length)
    {
        std::cerr << "Error in reading data (" << ec.value() << "): " << ec.message() << std::endl;
        exit(ec.value());
    }
    auto bufs = sb.data();
    auto body = std::string(asio::buffers_begin(bufs), asio::buffers_begin(bufs) + n);
    http_message.body = body;
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
 
