#pragma once

#include <string>
#include <sstream>
#include <map>

namespace LUrlParser{ class clParseURL; }
using HttpHeader = std::map<std::string, std::string>;

class HttpClient
{
public:
    struct RepliedMessage
    {
        std::string header, body, protocol, protocol_version;
        int status_code;
        std::string status_message;
        HttpHeader http_header;
        bool is_text_body;  //Weather or not the body is text or binary
    };
public:
    RepliedMessage sendGetCommand(const std::string& url, const std::string& header = "", bool verbose = false);
    RepliedMessage sendPostCommand(const std::string& url, const std::string& data = "",
                                const std::string& header = "", bool verbose = false);
private:
    bool isTextBody(const std::string& mime);
    RepliedMessage extractMessage(const std::string& message);
    void constructMessage(std::ostringstream& oss, LUrlParser::clParseURL& lurl, const std::string& header, const std::string& data);
};
