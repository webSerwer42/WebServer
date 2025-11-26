#include "Http.hpp"

void Http::parseRequestLine(const std::string &line)
{
    std::istringstream stream(line);
    stream >> requestMethod >> requestPath >> requestHttpVersion;
    return;
}

void Http::parseHeaderLine(const std::string &line)
{
    size_t delimiterPos = line.find(": ");
    if (delimiterPos == std::string::npos)
        return; // Invalid header line

    std::string key = line.substr(0, delimiterPos);
    std::string value = line.substr(delimiterPos + 2);
    requestHeaders[key] = value;
}

void Http::parseRequest(const std::string &rawRequest)
{
    if (rawRequest.empty())
    {
        return;
    }
    std::istringstream stream(rawRequest);
    std::string line;

    // Parse the request line
    if (!std::getline(stream, line) || line.empty())
    {
        return;
    }
    parseRequestLine(line);

    // Parse headers
    while (std::getline(stream, line) && !line.empty())
    {
        if (line == "\r")
        {
            break; // End of headers
        }
        parseHeaderLine(line);
    }

    // Parse body (if any)
    while (std::getline(stream, line))
    {
        requestBody += line + "\n";
    }
    return;
}

Http::Http(std::string &rawRequest, ServerConfig &serverData) : serverData(serverData)
{
    parseRequest(rawRequest);
}

void Http::getResponse()
{
    // this->requestHttpVersion = "HTTP/1.1";

    // // Szuka najlepszej (najdłuższy dopasowujący prefiks) lokacji, sprawdza czy GET jest dozwolone,
    // // sprawdza istnienie pliku i wczytuje go do requestBody (ustawia też status i nagłówki).
    // {
    //     using std::string;
    //     // brak std::filesystem i nowoczesnych udogodnień — implementacja zgodna z C++98

    //     int bestIndex = -1;
    //     size_t bestLen = 0;

    //     for (size_t i = 0; i < serverData.locations.size(); ++i) {
    //         const string &locPath = serverData.locations[i].path;
    //         if (requestPath.compare(0, locPath.size(), locPath) == 0 && locPath.size() >= bestLen) {
    //             bestLen = locPath.size();
    //             bestIndex = static_cast<int>(i);
    //         }
    //     }

    //     if (bestIndex < 0) {
    //         this->responseStatusCode = "404 Not Found";
    //         this->requestBody = "<html><body><h1>404 Not Found</h1></body></html>";
    //         return;
    //     }

    //     const typename std::vector< /* placeholder type */ >::value_type &location_dummy = *static_cast<typename std::vector< /* placeholder type */ >::const_pointer>(0);
    //     // Nie używamy tutaj location_dummy — tylko by zachować czytelność. 
    //     // Użyjemy bezpośrednio serverData.locations[bestIndex]

    //     const std::string &locPath = serverData.locations[bestIndex].path;
    //     const std::vector<std::string> &allowed = serverData.locations[bestIndex].allowedMethods;
    //     const std::string &root = serverData.locations[bestIndex].root;
    //     const std::vector<std::string> &indexList = serverData.locations[bestIndex].index;

    //     // czy GET jest dozwolone?
    //     bool getAllowed = false;
    //     for (size_t j = 0; j < allowed.size(); ++j) {
    //         if (allowed[j] == "GET") { getAllowed = true; break; }
    //     }
    //     if (!getAllowed) {
    //         this->responseStatusCode = "405 Method Not Allowed";
    //         this->requestBody = "<html><body><h1>405 Method Not Allowed</h1></body></html>";
    //         return;
    //     }

    //     // zbuduj ścieżkę pliku na dysku (prostym konkatenowaniem)
    //     string rel;
    //     if (requestPath.size() >= locPath.size())
    //         rel = requestPath.substr(locPath.size());
    //     else
    //         rel = "";

    //     if (rel.empty() || rel[rel.size() - 1] == '/') {
    //         if (!indexList.empty())
    //             rel += indexList[0];
    //         else
    //             rel += "index.html";
    //     }

    //     string fullPath = root;
    //     if (!fullPath.empty() && fullPath[fullPath.size() - 1] != '/' && fullPath[fullPath.size() - 1] != '\\')
    //         fullPath += '/';
    //     fullPath += rel;

    //     // spróbuj otworzyć plik — jeśli nie można, 404
    //     std::ifstream ifs(fullPath.c_str(), std::ios::binary);
    //     if (!ifs) {
    //         this->responseStatusCode = "404 Not Found";
    //         this->requestBody = "<html><body><h1>404 Not Found</h1></body></html>";
    //         return;
    //     }

    //     std::ostringstream ss;
    //     ss << ifs.rdbuf();
    //     this->requestBody = ss.str();
    //     this->responseStatusCode = "200 OK";

    //     // prosty mapper typów MIME (bez lambdy)
    //     string ext;
    //     size_t dotPos = fullPath.rfind('.');
    //     if (dotPos != string::npos)
    //         ext = fullPath.substr(dotPos);
    //     else
    //         ext = "";

    //     string mime;
    //     if (ext == ".html" || ext == ".htm") mime = "text/html";
    //     else if (ext == ".css") mime = "text/css";
    //     else if (ext == ".js") mime = "application/javascript";
    //     else if (ext == ".png") mime = "image/png";
    //     else if (ext == ".jpg" || ext == ".jpeg") mime = "image/jpeg";
    //     else if (ext == ".gif") mime = "image/gif";
    //     else mime = "application/octet-stream";

    //     // ustaw nagłówki (brak std::to_string — używamy ostringstream)
    //     std::ostringstream lenss;
    //     lenss << this->requestBody.size();
    //     this->responseHeaders["Content-Length"] = lenss.str();
    //     this->responseHeaders["Content-Type"] = mime;
    // }
}

void Http::postResponse()
{
    this->requestHttpVersion = "HTTP/1.1";
    this->responseStatusCode = "200 OK";
    this->requestBody = "<html><body><h1>POST request received</h1></body></html>";
}
void Http::deleteResponse()
{
    this->requestHttpVersion = "HTTP/1.1";
    this->responseStatusCode = "200 OK";
    this->requestBody = "<html><body><h1>DELETE request received</h1></body></html>";
}

void Http::methodNotAllowedResponse()
{
    this->requestHttpVersion = "HTTP/1.1";
    this->responseStatusCode = "405 Method Not Allowed";
    this->requestBody = "<html><body><h1>405 Method Not Allowed</h1></body></html>";
}

std::string Http::responseBuilder()
{
    if (this->requestMethod == "GET")
    {
        getResponse();
    }
    else if (this->requestMethod == "POST")
    {
        postResponse();
    }
    else if (this->requestMethod == "DELETE")
    {
        deleteResponse();
    }
    else
        methodNotAllowedResponse();
    return "";
}