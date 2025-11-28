#ifndef HTTP_HPP
#define HTTP_HPP

#include <string>
#include <map>
#include <vector>
#include <sstream>
#include <stdexcept>
#include "../configReader/config.hpp"
#include "../coreEngine/CoreEngine.hpp"

class Http {

    private:
        // server data
        ServerConfig serverData;

        // Request data
        std::string requestMethod;
        std::string requestPath;
        std::string requestHttpVersion;
        std::string requestBody;
        std::map<std::string, std::string> requestHeaders;
        
        // Response data
        std::map<std::string, std::string> responseHeaders;
        std::string responseStatusCode;
        std::string responseBody;

        // Parses a raw HTTP request string and returns an HttpRequest object
        void parseRequest(const std::string &rawRequest);
        
        // Helper functions for parsing
        void parseRequestLine(const std::string &line);
        void parseHeaderLine(const std::string &line);

        // Response functions
        void cgiResponse();
        void getResponse();
        void postResponse();
        void deleteResponse();

    public:

        // Constructor for creating HTTP responses
        Http(std::string& rawRequest, ServerConfig &serverConfig) : serverData(serverConfig) {
            parseRequest(rawRequest);
        }

        // Generates the HTTP response string
        std::string responseBuilder();
};

#endif