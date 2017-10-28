#include "http_server.h"
#include "../libhttpc/http_client.h"
#include "filesystem.h"
#include "request_handler.h"

#include <boost/filesystem.hpp>
#include <asio.hpp>
#include <iostream>
#include <thread>
#include <fstream>
#include <algorithm>

void readAndWrite(asio::ip::tcp::socket& active_socket)
{
    auto remote_address = active_socket.remote_endpoint().address();
    auto remote_port = active_socket.remote_endpoint().port();
    std::ostringstream oss;
    oss << remote_address << ':' << remote_port;
    auto remote_tcp_info = oss.str();

    if (verbose)
        std::cout << "Accept connection from " << remote_tcp_info << std::endl;

    std::string http_header;
    asio::streambuf buffer;
    auto n = asio::read_until(active_socket, buffer, "\r\n\r\n");
    auto iter = asio::buffers_begin(buffer.data());
    http_header = std::string(iter, iter + static_cast<long>(n));
    if (verbose)
        std::cout << "Received header (" << remote_tcp_info << "): " << std::endl
                  << http_header << std::endl;
    HttpMessage http_message = HttpClient::extractMessage(http_header, false);
    http_message.resource_path = url_decode(http_message.resource_path);
    std::string length_str = http_message.http_header["Content-Length"];
    unsigned int length = 0;
    if (!length_str.empty())
    {
        std::istringstream iss(length_str);
        iss >> length;
    }
    //We need to use the same buffer after consuming the header data:
    //It is important to remember that there may be additional data after the delimiter.
    //For more information: http://think-async.com/Asio/asio-1.11.0/doc/asio/overview/core/line_based.html
    buffer.consume(n);
    auto partial_len = std::min(static_cast<unsigned int>(buffer.size()), length);
    std::string body;
    if (partial_len > 0)
    {
        iter = asio::buffers_begin(buffer.data());
        body = std::string(iter, iter + partial_len);
        buffer.consume(partial_len);
    }
    auto num_bytes = length - body.length();
    if (num_bytes > 0)
    {
        n = asio::read(active_socket, buffer, asio::transfer_exactly(num_bytes));
        iter = asio::buffers_begin(buffer.data());
        body += std::string(iter, iter + static_cast<long>(num_bytes));
        buffer.consume(num_bytes);
    }
    http_message.body = body;
    if (verbose && http_message.is_text_body)
        std::cout << "Received body(" << remote_tcp_info
                  << "): "<< std::endl << body << std::endl;
    auto reply_msg = prepareReplyMessage(http_message);
    if (verbose)
    {
        HttpMessage tmp = HttpClient::extractMessage(reply_msg, false);
        std::cout << "Response message header (" << remote_tcp_info << "): " << tmp.header << std::endl;
        if (tmp.is_text_body)
        {
            tmp = HttpClient::extractMessage(reply_msg, true);
            std::cout << "Response message body (" << remote_tcp_info
                      << "): " << std::endl << tmp.body << std::endl;
        }
    }
    asio::write(active_socket, asio::buffer(reply_msg));
    if (buffer.size())
    {
        std::cerr << remote_tcp_info << " sent " << buffer.size() << " extra bytes!"
                  << std::endl;
    }
}

void handleClientHttpRequest(asio::ip::tcp::socket active_socket) noexcept
{
    try
    {
        readAndWrite(active_socket);
    }
    catch(const std::system_error& ex)
    {
        std::cerr << ex.what() << std::endl;
    }
    catch(...)
    {
        std::cerr << "Unknown exception for active socket " << std::endl;
    }
}

void runUdpServer(unsigned short port)
{
    using asio::ip::tcp;
    asio::io_service io_service;
    std::cout << "Listening on port " << port << std::endl;
    std::cout << "root directory is: " << root_dir_path << std::endl << std::endl;
    try
    {
        tcp::acceptor passive_socket(io_service, tcp::endpoint(tcp::v4(), port));
        while (true)
        {
            tcp::socket active_socket(io_service);
            passive_socket.accept(active_socket);
            std::thread(handleClientHttpRequest, std::move(active_socket)).detach();
        }
    }
    catch(const std::system_error& ex)
    {
        std::cerr << ex.what() << std::endl;
    }
    catch(...)
    {
        std::cerr << "Unknown exception for passive socket" << std::endl;
    }
}

