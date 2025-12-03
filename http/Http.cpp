#include "Http.hpp"
#include "../errors/error.hpp"

// Constructor
Http::Http (std::string &rawRequest, ServerConfig serverData){

    _rawRequestPtr = &rawRequest;
    _serverData = serverData;

    // Get location-specific configuration
    _myConfig = getMyConfig();
    // Initialize HttpError with server or location-specific error pages
    _httpError = HttpError(_myConfig.error_pages);

    requestBilder(rawRequest);
    responseBuilder();
    //testResponseBuilder();
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

bool Http::isCGI() {
// TODO: Implement
    return false;
}

bool Http::isMethodAllowed() {
    for (size_t i = 0; i < _myConfig.allow_methods.size(); ++i) {
        if (_s_requestData._method == _myConfig.allow_methods[i]) {
            return true;
        }
    }
    return false;
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
         // Wyświetl zawartość _myConfig
         html << "<h2>Location Config</h2>\n";
         html << "<p><strong>root:</strong> " << _myConfig.root << "</p>\n";
         html << "<p><strong>upload_dir:</strong> " << _myConfig.upload_dir << "</p>\n";
         html << "<p><strong>cgi_path:</strong> " << _myConfig.cgi_path << "</p>\n";
         html << "<p><strong>client_max_body_size:</strong> " << _myConfig.client_max_body_size << "</p>\n";
         html << "<p><strong>autoindex:</strong> " << (_myConfig.autoindex ? "true" : "false") << "</p>\n";

         html << "<p><strong>allow_methods:</strong></p>\n";
         for (size_t i = 0; i < _myConfig.allow_methods.size(); ++i)
             html << "<p> - " << _myConfig.allow_methods[i] << "</p>\n";

         html << "<p><strong>index:</strong></p>\n";
         for (size_t i = 0; i < _myConfig.index.size(); ++i)
             html << "<p> - " << _myConfig.index[i] << "</p>\n";

         html << "<p><strong>cgi_ext:</strong></p>\n";
         for (size_t i = 0; i < _myConfig.cgi_ext.size(); ++i)
             html << "<p> - " << _myConfig.cgi_ext[i] << "</p>\n";

         html << "<p><strong>error_pages:</strong></p>\n";
         for (std::map<int, std::string>::const_iterator it = _myConfig.error_pages.begin();
              it != _myConfig.error_pages.end(); ++it)
             html << "<p> - " << it->first << " => " << it->second << "</p>\n";
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

    if (!isBodySizeAllowed()) {
        _s_responseData._hasError = true;
        _s_responseData._responseStatusCode = 413;
        _s_responseData._response = _httpError.generateErrorResponse(413);
        return;
    }

    if (isCGI()) {
        cgiResponseBuilder();
        return;
    }
   if (!isMethodAllowed()) {
        _s_responseData._hasError = true;
        _s_responseData._responseStatusCode = 405;
        _s_responseData._response = _httpError.generateErrorResponse(405);
        return;
    }
    
    // Obsłuż według metody
    if (_s_requestData._method == "GET") {
        getResponseBuilder();
    } else if (_s_requestData._method == "POST") {
        postResponseBuilder();
    } else if (_s_requestData._method == "DELETE") {
        deleteResponseBuilder();
    } else {
        // Nieobsługiwana metoda
        _s_responseData._hasError = true;
        _s_responseData._responseStatusCode = 501;
        _s_responseData._response = _httpError.generateErrorResponse(501);
    }
}

LocationConfig Http::getMyConfig() {
    // Znajdź odpowiednią lokację dla ścieżki
    LocationConfig myConfig;
    bool locationFound = false;
    
    for (std::map<std::string, LocationConfig>::iterator it = _serverData.locations.begin();
         it != _serverData.locations.end(); ++it) {
        //czy ścieżka zaczyna się od route
        if (_s_requestData._path.find(it->first) == 0) {
            //merge
            myConfig = Config::getMergedLocationConfig(_serverData, it->second);
            locationFound = true;
            break;
        }
    }
    
    // Jeśli nie znaleziono lokacji, skopiuj ustawienia z serwera
    if (!locationFound) {
        myConfig.root = _serverData.root;
        myConfig.allow_methods = _serverData.allow_methods;
        myConfig.index = _serverData.index;
        myConfig.client_max_body_size = _serverData.client_max_body_size;
        myConfig.autoindex = _serverData.autoindex;
        myConfig.upload_dir = _serverData.upload_dir;
        myConfig.cgi_path = _serverData.cgi_path;
        myConfig.cgi_ext = _serverData.cgi_ext;
        myConfig.error_pages = _serverData.error_pages;
    }
    return myConfig;
}

bool Http::getIsError() const { return _s_responseData._hasError; }

int Http::getStatusCode() const { return _s_responseData._responseStatusCode; }

std::string Http::getResponse() const { return _s_responseData._response; }
