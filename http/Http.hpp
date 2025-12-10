#ifndef HTTP_HPP
#define HTTP_HPP

#include <string>
#include <map>
#include <vector>
#include <sstream>
#include <stdexcept>
#include <cstdlib>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <ctime>
#include <cstdio> // For remove
#include <cerrno> // For errno
#include "../configReader/config.hpp"
#include "../errors/error.hpp"

#define RESET       "\033[0m"
#define BLACK       "\033[30m"
#define RED         "\033[31m"
#define GREEN       "\033[32m"
#define YELLOW      "\033[33m"
#define BLUE        "\033[34m"
#define MAGENTA     "\033[35m"
#define CYAN        "\033[36m"
#define WHITE       "\033[37m"
#define ORANGE      "\033[38;5;208m"

class Http {
    
    private:
        ServerConfig _serverData;
        const std::string* _rawRequestPtr;  // wskaźnik do oryginalnego requesta
        size_t _bodyStart;
        size_t _bodyLen;
        HttpError _httpError;
        
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
            requestData() : 
                _contentLength(0),
                _Root(""),
                _Index(""),
                _Redirect(""),
                _CgiPath(""),
                _CgiExt(""),
                _UploadDir(""),
                Autoindex(false),
                _allowedMethods(),
                _rawHeader(""),
                _path(""),
                _httpVersion(""),
                _method(""),
                _headers()
                {}
            } _s_requestData;
            
            LocationConfig _myConfig;
            struct responseData {
                bool _hasError;
                std::string _responseHttpVersion;
            std::map<std::string, std::string> _responseHeaders;
            long long _clientMaxBodySize;
            std::string _responseHeader;
            int _responseStatusCode;
            std::string _responseBody;
            std::string _response;
            responseData() : 
                _hasError(false),
                _responseHttpVersion(""),
                _responseHeaders()  ,
                _clientMaxBodySize(0),
                _responseHeader(""),
                _responseStatusCode(0),
                _responseBody(""),
                _response("")
            {}
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
        void handleRootPath();
        void handleDirectory(const std::string& dirPath, const std::string& urlPath);
        void sendRedirect(const std::string& url, int code);
        bool resourceExists(const std::string& path);
        void sendError(int errorCode);
        bool isDirectory(const std::string& path);
        void handleFile(const std::string& filePath);
        std::string determineMimeType(const std::string& path);
        void generateDirectoryListing(const std::string& dirPath);
        void deleteFile(const std::string& filePath);
        void deleteDirectory(const std::string& dirPath);



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