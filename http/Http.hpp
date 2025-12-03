#ifndef HTTP_HPP
#define HTTP_HPP

#include <string>
#include <map>
#include <vector>
#include <sstream>
#include <stdexcept>
#include <cstdlib>
#include "../configReader/config.hpp"
#include "../errors/error.hpp"

class Http {
    
    private:
        ServerConfig _serverData;
        const std::string* _rawRequestPtr;  // wskaźnik do oryginalnego requesta
        size_t _bodyStart;
        size_t _bodyLen;
        HttpError _httpError;
        LocationConfig _myConfig;

        struct requestData {
            long long _contentLength;
            std::string _Root;
            std::string _Index;
            std::string _Redirect;
            std::string _CgiPath;
            std::string _CgiExt;
            std::string _UploadDir;
            bool Autoindex;
            std::vector<std::string> _allowedMethods;
            std::string _rawHeader;
            std::string _path;
            std::string _httpVersion;
            std::string _method;
            std::map<std::string, std::string> _headers;
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
        void parseRequest(std::string &rawRequest);
        void parseHeader();
        bool isMethodAllowed();
        bool isBodySizeAllowed();

        // Response functions
        void responseBuilder();
        void testResponseBuilder();
        void requestBilder(std::string &rawRequest);
        LocationConfig getMyConfig();
        bool isCGI();

        void cgiResponseBuilder();
        void getResponseBuilder();
        void postResponseBuilder();
        void deleteResponseBuilder();
        
    public:

        // Constructor for creating HTTP responses
        Http (std::string &rawRequest, ServerConfig serverData);

        // Generates the HTTP response string
        bool getIsError() const;
        int getStatusCode() const;
        std::string getResponse() const;
};

#endif