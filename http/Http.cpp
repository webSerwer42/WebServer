#include "Http.hpp"
#include "../errors/error.hpp"

// Constructor
Http::Http (std::string &rawRequest, ServerConfig serverData){

    _serverData = serverData;
    _rawRequestPtr = &rawRequest;

    requestBilder(rawRequest);
    //responseBuilder();
    testResponseBuilder();
}

void Http::requestBilder(std::string &rawRequest) {
    parseRequest(rawRequest);
    parseHeader();
}

// Parse HTTP request
void Http::parseRequest(std::string &rawRequest) {
    size_t headerEnd = rawRequest.find("\r\n\r\n");
    if (headerEnd != std::string::npos) {
        _s_requestData._rawHeader = rawRequest.substr(0, headerEnd);
        // Dla body - nie kopiuj, tylko zapisz pozycję_s_requestData.
        _rawRequestPtr = &rawRequest;
        _bodyStart = headerEnd + 4;
        _bodyLen = rawRequest.length() - (headerEnd + 4);
    } else {
        _s_requestData._rawHeader = rawRequest;

        // Debug output
        std::cout << "=== NO \\r\\n\\r\\n FOUND, RAW HEADER ===\n"
                    << _s_requestData._rawHeader
                    << "\n=== END ===" << std::endl;
        std::cout << "=== RAW REQUEST ===\n" 
                    << rawRequest
                    << "\n=== END RAW REQUEST ===" << std::endl;

        _rawRequestPtr = NULL;
        _bodyStart = 0;
        _bodyLen = 0;
    }

}

// Parse HTTP headers
void Http::parseHeader() {
    std::istringstream stream(_s_requestData._rawHeader);
    std::string line;

    // Parse request line
    if (std::getline(stream, line)) {
        std::istringstream lineStream(line);
        std::string method, path, version;     std::string _rawBody;
        
        lineStream >> method >> path >> version;
    
        // Usuń \r na końcu version jeśli istnieje
        if (!version.empty() && version[version.length() - 1] == '\r') {
            version.erase(version.length() - 1);
        }
        
        _s_requestData._method = method;
        _s_requestData._path = path;
        _s_requestData._httpVersion = version;
    }

    // Parse headers
    while (std::getline(stream, line)) {
        if (line == "\r" || line.empty()) {
            break; // End of headers
        }
        if (!line.empty() && line[line.length() - 1] == '\r') {
            line = line.substr(0, line.length() - 1); // Remove trailing \r
        }
        size_t delimiterPos = line.find(": ");
        if (delimiterPos != std::string::npos) {
            std::string key = line.substr(0, delimiterPos);
            std::string value = line.substr(delimiterPos + 2);
            _s_requestData._headers[key] = value;
        }
    }
}

void Http::parseConfigFile() {
    
}

bool Http::isBodySizeAllowed() {
    //TODO: Implement
    return true;
}

// Build CGI response
void Http::cgiResponseBuilder() {
    // TODO: Implement
}

// Build GET response
void Http::getResponseBuilder() {
    // TODO: Implement
}

// Build POST response
void Http::postResponseBuilder() {
    // TODO: Implement
}

// Build DELETE response
void Http::deleteResponseBuilder() {
    // TODO: Implement
}

bool Http::isMethodAllowed(const std::string& method) {
    // Iteruj przez wektor i szukaj metody
    for (std::vector<std::string>::const_iterator it = _s_requestData._allowedMethods.begin(); 
         it != _s_requestData._allowedMethods.end(); ++it)
        {
        if (*it == method)
            return true;
    }
    return false; // Metoda nie jest dozwolona
}

void Http::testResponseBuilder() {
//tempolary response for testing
    if (_s_requestData._path == "/favicon.ico") {
        _s_responseData._response = "HTTP/1.1 204 No Content\r\n"
                                   "Connection: close\r\n"
                                   "\r\n";
        return;
    }

    std::ostringstream html;

    html << "HTTP/1.1 200 OK\r\n"
         << "Content-Type: text/html; charset=UTF-8\r\n"
         << "Connection: close\r\n"
         << "\r\n"
         << "<!DOCTYPE html>\n"
         << "<html>\n"
         << "<head>\n"
         << "<meta charset=\"UTF-8\">\n"
         << "<title>" << 200 << " " << "OK" << "</title>\n"
         << "</head>\n"
         << "<body>\n"
         << "<h1>" << 200 << " " << "OK" << "</h1>\n"
         << "<p>" << "Request was successful." << "</p>\n"
         << "<p>Method: " << _s_requestData._method << "</p>\n"
         << "<p>Path: " << _s_requestData._path << "</p>\n"
         << "<p>Version: " << _s_requestData._httpVersion << "</p>\n"
         << "<p>Headers:</p>\n";
         for (std::map<std::string, std::string>::const_iterator it = _s_requestData._headers.begin(); 
              it != _s_requestData._headers.end(); ++it)
         {
             html << "<p><strong>" << it->first << ":</strong> " << it->second << "</p>\n";
             std::cout << "Header: " << it->first << " => " << it->second << std::endl;
         }
         html << "</body>\n"
              << "</html>";

    _s_responseData._response = html.str();
}

void Http::responseBuilder() {

    if (_s_requestData._path == "/favicon.ico") {
        _s_responseData._response = "HTTP/1.1 204 No Content\r\n"
                                   "Connection: close\r\n"
                                   "\r\n";
        return;
    }

    if (_s_requestData._method == "GET" && isMethodAllowed("GET")) {
        getResponseBuilder();
    } else if (_s_requestData._method == "POST" && isMethodAllowed("POST")) {
        postResponseBuilder();
    } else if (_s_requestData._method == "DELETE" && isMethodAllowed("DELETE")) {
        deleteResponseBuilder();
    } else {
        // Method not allowed
        _s_responseData._hasError = true;
        _s_responseData._responseStatusCode = 405;
        _s_responseData._responseBody = "405 Method Not Allowed";
        HttpError error;
        _s_responseData._response = error.generateErrorResponse(405);
    }
}


bool Http::getIsError() const { return _s_responseData._hasError; }

int Http::getStatusCode() const { return _s_responseData._responseStatusCode; }

std::string Http::getResponse() const { return _s_responseData._response; }
