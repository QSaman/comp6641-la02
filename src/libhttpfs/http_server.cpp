#include "http_server.h"
#include "../libhttpc/http_client.h"
#include "filesystem.h"

#include <boost/filesystem.hpp>
#include <asio.hpp>
#include <iostream>
#include <thread>
#include <fstream>

std::string root_dir_path = "./resources/https_root";
static int level = 0;

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
    if (client_msg.resource_path == "/")
    {
        try
        {
            std::string body = jsonDirList(root_dir_path, level);
            std::string partial_header = "HTTP/1.1 200 OK\r\nContent-Type: application/json";
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
    using namespace boost::filesystem;
    path dir_path(root_dir_path);
    path file_path(client_msg.resource_path);
    try
    {
        std::string path = (dir_path / file_path).c_str();
        std::string body = fileContent(path);
        std::string partial_header = "HTTP/1.1 200 OK\r\nContent-Type: text/plain";
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

void handleClientHttpRequest(asio::ip::tcp::socket active_socket)
{
    asio::error_code ec;
    std::string http_header;

    while (true)
    {
        asio::streambuf header_sb(4);
        auto n = asio::read(active_socket, header_sb, ec);
        if (ec && ec != asio::error::eof)
        {
            std::cerr << "Error in reading data (" << ec.value() << "): " << ec.message() << std::endl;
            exit(ec.value());
        }
        auto bufs = header_sb.data();
        auto tmp = std::string(asio::buffers_begin(bufs), asio::buffers_begin(bufs) + n);
        if (tmp == "\r\n\r\n" || tmp.empty())
            break;
        http_header += tmp;
    }
    std::cout << "header:" << std::endl << http_header << std::endl;
    HttpMessage http_message = HttpClient::extractMessage(http_header, false);
    std::string length_str = http_message.http_header["Content-Length"];
    std::istringstream iss(length_str);
    unsigned int length;
    iss >> length;
    asio::streambuf body_sb(length);
    auto n = asio::read(active_socket, body_sb, ec);
    if ((ec && ec != asio::error::eof) || n != length)
    {
        std::cerr << "Error in reading data (" << ec.value() << "): " << ec.message() << std::endl;
        exit(ec.value());
    }
    auto bufs = body_sb.data();
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
    std::cout << "Listening on port " << port << std::endl;
    tcp::acceptor passive_socket(io_service, tcp::endpoint(tcp::v4(), port));
    while (true)
    {
        tcp::socket active_socket(io_service);
        passive_socket.accept(active_socket);
        std::thread(handleClientHttpRequest, std::move(active_socket)).detach();
    }
}

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
    if (!body.empty())
    {
        oss << "\r\nContent-Length: " << body.length();
        oss << "\r\n\r\n" << body;
    }
    return oss.str();
}
