#include "Http.hpp"
#include "../errors/error.hpp"
#include <cstdlib> // For std::atoi

// Constructor
Http::Http (std::string &rawRequest, ServerConfig serverData){

    // Initialize primitive types
    _rawRequestPtr = NULL;
    _bodyStart = 0;
    _bodyLen = 0;
    

    //std::cout << "Debug: Entering Http constructor" << std::endl;

    _rawRequestPtr = &rawRequest;
    _serverData = serverData;

    // Get location-specific configuration
    requestBilder(rawRequest);
    _myConfig = getMyConfig();
    
    // Debug - sprawdź wartości po getMyConfig()
    std::cout << YELLOW << "DEBUG _myConfig.has_redirect: " << _myConfig.has_redirect << RESET << std::endl;
    std::cout << YELLOW << "DEBUG _myConfig.redirect_url: '" << _myConfig.redirect_url << "'" << RESET << std::endl;
    std::cout << YELLOW << "DEBUG _myConfig.redirect_code: " << _myConfig.redirect_code << RESET << std::endl;
    
    // Initialize HttpError with server or location-specific error pages
    _httpError = HttpError(_myConfig.error_pages);

    responseBuilder();
    //testResponseBuilder();
    std::cout << GREEN << "Debug: Exiting Http constructor" << std::endl;
    std::cout << _s_responseData._response << RESET << std::endl;
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
        std::cout << RED << "=== NO \\r\\n\\r\\n FOUND, RAW HEADER ===\n"
                    << _s_requestData._rawHeader
                    << "\n=== END ===" << std::endl;
        std::cout << "=== RAW REQUEST ===\n" 
                    << rawRequest
                    << "\n=== END RAW REQUEST ===" << RESET << std::endl;
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
        std::string method, path, version;
        
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
    std::map<std::string, std::string>::iterator it = _s_requestData._headers.find("Content-Length");
    if (it != _s_requestData._headers.end()) {
        size_t contentLength = static_cast<size_t>(std::atoi(it->second.c_str()));
        size_t maxBodySize = static_cast<size_t>(std::atoi(_myConfig.client_max_body_size.c_str()));
        if (contentLength > maxBodySize) {
            return false;
        }
    }
    return true;
}

// Build CGI response
void Http::cgiResponseBuilder() {
    // Usuń query string ze ścieżki
    std::string cleanPath = _s_requestData._path;
    size_t queryPos = cleanPath.find('?');
    std::string queryString;
    
    if (queryPos != std::string::npos) {
        queryString = cleanPath.substr(queryPos + 1);
        cleanPath = cleanPath.substr(0, queryPos);
    }
    
    // Usuń location_path z URI
    std::string filePath = cleanPath;
    if (!_myConfig.location_path.empty() && cleanPath.find(_myConfig.location_path) == 0) {
        filePath = cleanPath.substr(_myConfig.location_path.length());
    }
    
    // Zbuduj pełną ścieżkę do skryptu
    std::string scriptPath = _myConfig.root + filePath;
    
    // Sprawdź czy skrypt istnieje
    if (!resourceExists(scriptPath)) {
        sendError(404);
        return;
    }
    
    // Pobierz body (dla POST)
    std::string body;
    if (_rawRequestPtr != NULL && _bodyLen > 0) {
        body = _rawRequestPtr->substr(_bodyStart, _bodyLen);
    }
    
    // Przygotuj zmienne środowiskowe
    std::map<std::string, std::string> envVars;
    
    envVars["REQUEST_METHOD"] = _s_requestData._method;
    envVars["SCRIPT_FILENAME"] = scriptPath;
    envVars["QUERY_STRING"] = queryString;
    envVars["SERVER_PROTOCOL"] = _s_requestData._httpVersion;
    envVars["PATH_INFO"] = filePath;
    envVars["SCRIPT_NAME"] = cleanPath;
    
    // Content-Length dla POST
    if (!body.empty()) {
        std::ostringstream oss;
        oss << body.length();
        envVars["CONTENT_LENGTH"] = oss.str();
    }
    
    // Content-Type z headerów
    std::map<std::string, std::string>::iterator it = 
        _s_requestData._headers.find("Content-Type");
    if (it != _s_requestData._headers.end()) {
        envVars["CONTENT_TYPE"] = it->second;
    }
    
    // Dodaj inne headery jako HTTP_*
    for (std::map<std::string, std::string>::iterator it = _s_requestData._headers.begin();
         it != _s_requestData._headers.end(); ++it) {
        std::string key = "HTTP_" + it->first;
        // Zamień '-' na '_' i uppercase
        for (size_t i = 0; i < key.length(); ++i) {
            if (key[i] == '-') key[i] = '_';
            key[i] = std::toupper(key[i]);
        }
        envVars[key] = it->second;
    }
    
    // Wykonaj CGI
    std::string response = executeCgi(scriptPath, _myConfig.cgi_path, body, envVars);
    
    _s_responseData._response = response;
    _s_responseData._responseStatusCode = 200;  
}

// Build GET response
void Http::getResponseBuilder() {
// NAJPIERW sprawdź redirect z configu
    if (_myConfig.has_redirect) {
        sendRedirect(_myConfig.redirect_url, _myConfig.redirect_code);
        return;
    }

// Usuń query string ze ścieżki
    std::string cleanPath = _s_requestData._path;
    size_t queryPos = cleanPath.find('?');
    if (queryPos != std::string::npos) {
        cleanPath = cleanPath.substr(0, queryPos);
    }
    
// Krok 1: Obsługa głównej ścieżki "/"
    cleanPath = cleanPath.empty() ? "/" : cleanPath;
    // Zła obsługa Route miesza rout z katalogami serwera.
    if (cleanPath == "/") {
        handleRootPath();
        return;
    }
    
    // Krok 2: Zbuduj pełną ścieżkę
    // Usuwa location_path z URI, żeby zostać z rzeczywistą ścieżką pliku
    std::string test1 = _myConfig.location_path;
    std::string filePath = cleanPath;
    if (!_myConfig.location_path.empty() && cleanPath.find(_myConfig.location_path) == 0) {
        // Usuń location_path z początku
        filePath = cleanPath.substr(_myConfig.location_path.length());
    }

    // Jeśli filePath jest pusta lub to tylko "/", obsłuż jako root lokacji
    if (filePath.empty() || filePath == "/") {
        handleRootPath();
        return;
    }

    std::string fullPath = _myConfig.root + filePath;
    
    // Krok 3: Sprawdź czy zasób istnieje
    if (!resourceExists(fullPath)) {
        sendError(404);
        return;
    }
    
    // Krok 4: Sprawdź typ zasobu
    if (isDirectory(fullPath)) {
        handleDirectory(fullPath, cleanPath);
    } else {
        handleFile(fullPath);
    }
}

bool Http::isDirectory(const std::string& path) {
    //stat() - dozwolona funkcja
    struct stat buffer;
    if (stat(path.c_str(), &buffer) != 0) {
        return false;
    }
    return S_ISDIR(buffer.st_mode);
}

void Http::sendError(int errorCode) {
    _s_responseData._hasError = true;
    _s_responseData._responseStatusCode = errorCode;
    _s_responseData._response = _httpError.generateErrorResponse(errorCode);
}



bool Http::resourceExists(const std::string& path) {
    return (access(path.c_str(), F_OK) == 0);
}

void Http::sendRedirect(const std::string& newLocation, int code) {
    // Domyślnie 301 jeśli nie podano
    if (code == 0) {
        code = 301;
    }
    
    _s_responseData._responseStatusCode = code;
    std::ostringstream response;
    
    std::string statusText;
    switch (code) {
        case 301: statusText = "Moved Permanently"; break;
        case 302: statusText = "Found"; break;
        case 303: statusText = "See Other"; break;
        case 307: statusText = "Temporary Redirect"; break;
        case 308: statusText = "Permanent Redirect"; break;
        default: statusText = "Moved Permanently"; code = 301; break;
    }
    
    response << "HTTP/1.1 " << code << " " << statusText << "\r\n"
             << "Location: " << newLocation << "\r\n"
             << "Connection: close\r\n"
             << "\r\n";
    
    _s_responseData._response = response.str();
}

std::string Http::determineMimeType(const std::string& path) {
    size_t dotPos = path.rfind('.');
    if (dotPos == std::string::npos) {
        return "application/octet-stream";
    }
    
    std::string extension = path.substr(dotPos);
    
    if (extension == ".html" || extension == ".htm")
        return "text/html";
    if (extension == ".css")
        return "text/css";
    if (extension == ".js")
        return "application/javascript";
    if (extension == ".png")
        return "image/png";
    if (extension == ".jpg" || extension == ".jpeg")
        return "image/jpeg";
    if (extension == ".gif")
        return "image/gif";
    if (extension == ".txt")
        return "text/plain";
    
    return "application/octet-stream";
}

void Http::handleFile(const std::string& filePath) {
    int fd = open(filePath.c_str(), O_RDONLY);
    
    if (fd == -1) {
        sendError(403);
        return;
    }
    
    struct stat fileStat;
    if (fstat(fd, &fileStat) == -1) {
        close(fd);
        sendError(500);
        return;
    }
    
    char* buffer = new char[fileStat.st_size + 1];
    ssize_t bytesRead = read(fd, buffer, fileStat.st_size);
    close(fd);
    
    if (bytesRead == -1) {
        delete[] buffer;
        sendError(500);
        return;
    }
    
    buffer[bytesRead] = '\0';
    std::string body(buffer, bytesRead);
    delete[] buffer;
    
    std::string mimeType = determineMimeType(filePath);
    
    // Zbuduj odpowiedź
    std::ostringstream response;
    response << "HTTP/1.1 200 OK\r\n"
             << "Content-Type: " << mimeType << "\r\n"
             << "Content-Length: " << body.length() << "\r\n"
             << "Connection: close\r\n"
             << "\r\n"
             << body;
    
    _s_responseData._response = response.str();
    _s_responseData._responseStatusCode = 200;
}

void Http::generateDirectoryListing(const std::string& dirPath) {
    DIR* dir = opendir(dirPath.c_str());
    if (!dir) {
        sendError(500);
        return;
    }
    
    std::ostringstream html;
    html << "<html>\n<head>\n<title>Index of " << _s_requestData._path << "</title>\n</head>\n"
         << "<body>\n<h1>Index of " << _s_requestData._path << "</h1>\n<hr>\n<ul>\n";
    
    // Dodaj link do katalogu nadrzędnego
    if (_s_requestData._path != "/") {
        html << "<li><a href=\"..\">../</a></li>\n";
    }
    
    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL) {
        std::string name = entry->d_name;
        
        // Pomiń . i ..
        if (name == "." || name == "..") {
            continue;
        }
        
        std::string fullPath = dirPath + "/" + name;
        
        // Sprawdź czy to katalog
        if (isDirectory(fullPath)) {
            html << "<li><a href=\"" << name << "/\">" << name << "/</a></li>\n";
        } else {
            html << "<li><a href=\"" << name << "\">" << name << "</a></li>\n";
        }
    }
    
    closedir(dir); // ✅ closedir() - dozwolona funkcja
    
    html << "</ul>\n<hr>\n</body>\n</html>";
    
    std::string body = html.str();
    std::ostringstream response;
    response << "HTTP/1.1 200 OK\r\n"
             << "Content-Type: text/html\r\n"
             << "Content-Length: " << body.length() << "\r\n"
             << "Connection: close\r\n"
             << "\r\n"
             << body;
    
    _s_responseData._response = response.str();
    _s_responseData._responseStatusCode = 200;
}

void Http::handleRootPath() {
    // Jeśli jest skonfigurowany index
    if (!_myConfig.index.empty()) {
        std::string indexPath = _myConfig.root + "/" + _myConfig.index;
        
        if (resourceExists(indexPath)) {
            handleFile(indexPath);
            return;
        }
    }
    // Brak index - sprawdź autoindex
    if (_myConfig.autoindex) {
        generateDirectoryListing(_myConfig.root);
    } else {
        sendError(403);
    }
}

void Http::handleDirectory(const std::string& dirPath, const std::string& urlPath) {
    // Czy URL kończy się na '/'?
    if (urlPath[urlPath.length() - 1] != '/') {
        // Jeśli nie, dodaj '/' na końcu
        // przekieruj
        sendRedirect(urlPath + "/", 301);
        return;
    }
    
    // Szukaj pliku index
    if (!_myConfig.index.empty()) {
        std::string indexPath = dirPath + "/" + _myConfig.index;
        
        if (resourceExists(indexPath)) {
            handleFile(indexPath);
            return;
        }
    }
    
    // Nie ma index - sprawdź autoindex
    if (_myConfig.autoindex) {
        generateDirectoryListing(dirPath);
    } else {
        sendError(403);
    }
}


// Build POST response
void Http::postResponseBuilder() {
    // Sprawdź czy mamy body
    if (_rawRequestPtr == NULL || _bodyLen == 0) {
        sendError(400); // Bad Request - brak danych
        return;
    }
    
    // Pobierz body
    std::string body = _rawRequestPtr->substr(_bodyStart, _bodyLen);
    
    // Jeśli location ma upload_dir - zapisz jako plik
    if (!_myConfig.upload_dir.empty()) {
        // UPLOAD PLIKU
        
        // Wygeneruj unikalną nazwę
        std::ostringstream oss;
        oss << "upload_" << std::time(NULL) << ".bin";
        std::string filename = oss.str();
        
        // Zbuduj pełną ścieżkę
        std::string fullPath = _myConfig.upload_dir + "/" + filename;
        
        // Zapisz plik
        int fd = open(fullPath.c_str(), O_WRONLY | O_CREAT | O_EXCL, 0644);
        if (fd == -1) {
            sendError(500);
            return;
        }
        
        ssize_t bytesWritten = write(fd, body.c_str(), body.length());
        close(fd);
        
        if (bytesWritten == -1 || static_cast<size_t>(bytesWritten) != body.length()) {
            std::remove(fullPath.c_str());
            sendError(500);
            return;
        }
        
        // Sukces - 201 Created
        std::ostringstream response;
        response << "HTTP/1.1 201 Created\r\n"
                 << "Content-Type: text/html\r\n"
                 << "Location: " << _myConfig.upload_dir << "/" << filename << "\r\n"
                 << "Connection: close\r\n"
                 << "\r\n"
                 << "<html><body>"
                 << "<h1>File Uploaded</h1>"
                 << "<p>Filename: " << filename << "</p>"
                 << "<p>Size: " << body.length() << " bytes</p>"
                 << "</body></html>";
        
        _s_responseData._response = response.str();
        _s_responseData._responseStatusCode = 201;
        
    } else {
        // ZWYKŁY POST (bez uploadu)
        
        std::ostringstream response;
        response << "HTTP/1.1 200 OK\r\n"
                 << "Content-Type: text/html\r\n"
                 << "Connection: close\r\n"
                 << "\r\n"
                 << "<html><body>"
                 << "<h1>POST Data Received</h1>"
                 << "<p>Content-Length: " << body.length() << " bytes</p>"
                 << "<pre>" << body << "</pre>"
                 << "</body></html>";
        
        _s_responseData._response = response.str();
        _s_responseData._responseStatusCode = 200;
    }
}

// Build DELETE response
void Http::deleteResponseBuilder() {
    // Usuń query string
    std::string cleanPath = _s_requestData._path;
    size_t queryPos = cleanPath.find('?');
    if (queryPos != std::string::npos) {
        cleanPath = cleanPath.substr(0, queryPos);
    }
    
    // Usuń location_path z URI
    std::string filePath = cleanPath;
    if (!_myConfig.location_path.empty() && cleanPath.find(_myConfig.location_path) == 0) {
        filePath = cleanPath.substr(_myConfig.location_path.length());
    }
    
    // Zbuduj pełną ścieżkę
    std::string fullPath = _myConfig.root + filePath;
    
    // Sprawdź czy zasób istnieje
    if (!resourceExists(fullPath)) {
        sendError(404);
        return;
    }
    
    // Sprawdź typ zasobu
    if (isDirectory(fullPath)) {
        deleteDirectory(fullPath);
    } else {
        deleteFile(fullPath);
    }
}

void Http::deleteFile(const std::string& filePath) {

    // Sprawdź czy ścieżka zawiera ".."
    if (filePath.find("..") != std::string::npos) {
        sendError(403); // Path traversal
        return;
    }
    
    // Sprawdź czy plik jest w root
    if (filePath.find(_myConfig.root) != 0) {
        sendError(403); // Próba wyjścia poza root
        return;
    }

    // Sprawdź uprawnienia do zapisu (access dozwolone)
    if (access(filePath.c_str(), W_OK) != 0) {
        sendError(403); // Forbidden
        return;
    }
    
    // Usuń plik (unlink dozwolone w C - możesz użyć przez std::remove)
    if (std::remove(filePath.c_str()) != 0) {
        // Błąd usuwania
        sendError(500);
        return;
    }
    
    // Sukces - wyślij 204 No Content
    std::ostringstream response;
    response << "HTTP/1.1 204 No Content\r\n"
             << "Connection: close\r\n"
             << "\r\n";
    
    _s_responseData._response = response.str();
    _s_responseData._responseStatusCode = 204;
}

void Http::deleteDirectory(const std::string& dirPath) {
    // Sprawdź czy URL kończy się na '/'
    std::string urlPath = _s_requestData._path;
    if (urlPath[urlPath.length() - 1] != '/') {
        sendError(409); // Conflict - wymagany trailing slash dla katalogów
        return;
    }
    
    // Sprawdź uprawnienia
    if (access(dirPath.c_str(), W_OK) != 0) {
        sendError(403);
        return;
    }
    
    // Spróbuj usunąć katalog (tylko pusty)
    if (rmdir(dirPath.c_str()) != 0) {
        if (errno == ENOTEMPTY || errno == EEXIST) {
            sendError(409); // Conflict - katalog nie jest pusty
        } else {
            sendError(500);
        }
        return;
    }
    
    // Sukces
    std::ostringstream response;
    response << "HTTP/1.1 204 No Content\r\n"
             << "Connection: close\r\n"
             << "\r\n";
    
    _s_responseData._response = response.str();
    _s_responseData._responseStatusCode = 204;
}

bool Http::isCGI() {
    // Sprawdź czy CGI jest skonfigurowane
    if (_myConfig.cgi_ext.empty() || _myConfig.cgi_path.empty()) {
        return false;
    }
    
    // Pobierz rozszerzenie z path
    size_t dotPos = _s_requestData._path.rfind('.');
    if (dotPos == std::string::npos) {
        return false;
    }
    
    std::string extension = _s_requestData._path.substr(dotPos);
    
    // Sprawdź czy rozszerzenie pasuje
    // UWAGA: _myConfig.cgi_ext to string, nie vector!
    return (extension == _myConfig.cgi_ext);
}

bool Http::isMethodAllowed() {
    for (size_t i = 0; i < _myConfig.allow_methods.size(); ++i) {
        if (_s_requestData._method == _myConfig.allow_methods[i]) {
            return true;
        }
    }
    return false;
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
    
    // Wyzeruj wszystkie wartości
    myConfig.location_path = "";
    myConfig.root = "";
    myConfig.index = "";
    myConfig.autoindex = false;
    myConfig.client_max_body_size = "";
    myConfig.upload_dir = "";
    myConfig.cgi_path = "";
    myConfig.cgi_ext = "";
    myConfig.has_redirect = false;
    myConfig.redirect_url = "";
    myConfig.redirect_code = 0;
    myConfig.allow_methods.clear();
    myConfig.error_pages.clear();
    
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
