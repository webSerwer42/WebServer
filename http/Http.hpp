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
    
    
    std::string _rawRequest;
    ServerConfig _serverData;
    
    struct requestData {
        long long _contentLength;
        std::vector<std::string> _allowedMethods;
        std::string _rawHeader;
        std::string _rawBody;
        std::string _path;
        std::string _httpVersion;
        std::map<std::string, std::string> _headers;
        std::string _body;
    } _s_requestData;
        
    struct responseData {
        bool _hasError;
        std::string _responseHttpVersion;
        std::map<std::string, std::string> _responseHeaders;
        long long _clientMaxBodySize;
        std::string _responseHeader;
        int _responseStatusCode;
        std::string _responseBody;
        std::string _response;
    } _s_responseData;

        // Parses a raw HTTP request.
        void parseRequest();
        void parseHeader();

        // Response functions
        void responseBuilder();

        void cgiResponseBuilder();
        void getResponseBuilder();
        void postResponseBuilder();
        void deleteResponseBuilder();
        
    public:

        // Constructor for creating HTTP responses
        Http (std::string rawRequest, ServerConfig serverData, bool hasError);

        // Generates the HTTP response string
        bool getIsError() const;
        int getStatusCode() const;
        std::string getResponse() const;
};

#endif