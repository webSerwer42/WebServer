#ifndef HTTP_HPP
#define HTTP_HPP

#include <string>
#include <map>
#include <vector>
#include <sstream>
#include <stdexcept>
#include "../configReader/config.hpp"

class Http {

    private:
        // Reference to server configuration
        ServerConfig &serverData;
        std::string rawRequestHeader;
        std::string rawRequestBody;

        // Request data
        struct requestData {
            long long contentLength;
            std::string method;
            std::string path;
            std::string httpVersion;
            std::map<std::string, std::string> headers;
            std::string body;
        } requestData;
        
        // Response data
        struct responseData {
            std::string responseHttpVersion;
            std::map<std::string, std::string> responseHeaders;
            long long clientMaxBodySize;
            std::string responseHeader;
            std::string responseStatusCode;
            std::string responseBody;
            std::string response;
        } responseData;

        // Parses a raw HTTP request.
        void parseRequest(const std::string &rawRequest);
        void parseHeader(const std::string &rawHeader);

        // Response functions
        void cgiResponseBuilder();
        void getResponseBuilder();
        void postResponseBuilder();
        void deleteResponseBuilder();

    public:

        // Constructor for creating HTTP responses
        Http(std::string& rawRequest, ServerConfig &serverData);

        // Generates the HTTP response string
        std::string responseBuilder();
};

#endif