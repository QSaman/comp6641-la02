#pragma once

#include <string>
#include <sstream>
#include <map>

namespace LUrlParser{ class clParseURL; }
using HttpHeader = std::map<std::string, std::string>;

enum class HttpMessageType
{
    Reply,
    Get,
    Post
};

struct HttpMessage
{
    HttpMessageType message_type;
    std::string header, body, protocol, protocol_version;
    int status_code;
    std::string resource_path;
    std::string status_message;
    HttpHeader http_header;
    bool is_text_body;  //Weather or not the body is text or binary
};

class HttpClient
{
public:
    HttpMessage sendGetCommand(const std::string& url, const std::string& header = "", bool verbose = false);
    HttpMessage sendPostCommand(const std::string& url, const std::string& data = "",
                                const std::string& header = "", bool verbose = false);
    static HttpMessage extractMessage(const std::string& message, bool read_body = true);
    static bool isTextBody(const std::string& mime);
private:    
    void constructMessage(std::ostringstream& oss, LUrlParser::clParseURL& lurl, const std::string& header, const std::string& data);
};
