/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   error.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: agorski <agorski@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/26 14:37:10 by agorski           #+#    #+#             */
/*   Updated: 2025/11/26 15:27:35 by agorski          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "error.hpp"
#include <fstream>
#include <sys/stat.h>
#include <unistd.h>

HttpError::HttpError() {
    initializeErrorMaps();
}

void HttpError::initializeErrorMaps() {
    // ============ 4xx CLIENT ERRORS (z MDN) ============
    
    errorMap[400] = "Bad Request";
    errorDescriptions[400] = "The server cannot process the request due to malformed syntax.";
    
    errorMap[401] = "Unauthorized";
    errorDescriptions[401] = "Authentication is required to access this resource.";
    
    errorMap[402] = "Payment Required";
    errorDescriptions[402] = "Payment is required to access this resource.";
    
    errorMap[403] = "Forbidden";
    errorDescriptions[403] = "The server refuses to authorize the request.";
    
    errorMap[404] = "Not Found";
    errorDescriptions[404] = "The requested resource could not be found on this server.";
    
    errorMap[405] = "Method Not Allowed";
    errorDescriptions[405] = "The request method is not supported for this resource.";
    
    errorMap[406] = "Not Acceptable";
    errorDescriptions[406] = "The server cannot produce a response matching the Accept headers.";
    
    errorMap[407] = "Proxy Authentication Required";
    errorDescriptions[407] = "Authentication with the proxy is required.";
    
    errorMap[408] = "Request Timeout";
    errorDescriptions[408] = "The server timed out waiting for the request.";
    
    errorMap[409] = "Conflict";
    errorDescriptions[409] = "The request conflicts with the current state of the server.";
    
    errorMap[410] = "Gone";
    errorDescriptions[410] = "The requested resource is no longer available and will not be available again.";
    
    errorMap[411] = "Length Required";
    errorDescriptions[411] = "The request must specify its content length.";
    
    errorMap[412] = "Precondition Failed";
    errorDescriptions[412] = "One or more preconditions in the request headers were false.";
    
    errorMap[413] = "Content Too Large";
    errorDescriptions[413] = "The request payload is larger than the server is willing to process.";
    
    errorMap[414] = "URI Too Long";
    errorDescriptions[414] = "The URI requested is longer than the server can interpret.";
    
    errorMap[415] = "Unsupported Media Type";
    errorDescriptions[415] = "The media format of the requested data is not supported.";
    
    errorMap[416] = "Range Not Satisfiable";
    errorDescriptions[416] = "The range specified in the Range header cannot be fulfilled.";
    
    errorMap[417] = "Expectation Failed";
    errorDescriptions[417] = "The expectation given in the Expect header could not be met.";
    
    errorMap[418] = "I'm a teapot";
    errorDescriptions[418] = "The server refuses to brew coffee because it is a teapot (RFC 2324).";
    
    errorMap[421] = "Misdirected Request";
    errorDescriptions[421] = "The request was directed at a server that is not able to produce a response.";
    
    errorMap[422] = "Unprocessable Content";
    errorDescriptions[422] = "The request was well-formed but contains semantic errors.";
    
    errorMap[423] = "Locked";
    errorDescriptions[423] = "The resource being accessed is locked.";
    
    errorMap[424] = "Failed Dependency";
    errorDescriptions[424] = "The request failed due to failure of a previous request.";
    
    errorMap[425] = "Too Early";
    errorDescriptions[425] = "The server is unwilling to risk processing a request that might be replayed.";
    
    errorMap[426] = "Upgrade Required";
    errorDescriptions[426] = "The client should switch to a different protocol.";
    
    errorMap[428] = "Precondition Required";
    errorDescriptions[428] = "The origin server requires the request to be conditional.";
    
    errorMap[429] = "Too Many Requests";
    errorDescriptions[429] = "The user has sent too many requests in a given amount of time.";
    
    errorMap[431] = "Request Header Fields Too Large";
    errorDescriptions[431] = "The server refuses to process the request because header fields are too large.";
    
    errorMap[451] = "Unavailable For Legal Reasons";
    errorDescriptions[451] = "The resource is unavailable for legal reasons.";
    
    // ============ 5xx SERVER ERRORS (z MDN) ============
    
    errorMap[500] = "Internal Server Error";
    errorDescriptions[500] = "The server encountered an unexpected condition that prevented it from fulfilling the request.";
    
    errorMap[501] = "Not Implemented";
    errorDescriptions[501] = "The server does not support the functionality required to fulfill the request.";
    
    errorMap[502] = "Bad Gateway";
    errorDescriptions[502] = "The server received an invalid response from an upstream server.";
    
    errorMap[503] = "Service Unavailable";
    errorDescriptions[503] = "The server is currently unable to handle the request due to temporary overload or maintenance.";
    
    errorMap[504] = "Gateway Timeout";
    errorDescriptions[504] = "The server did not receive a timely response from an upstream server.";
    
    errorMap[505] = "HTTP Version Not Supported";
    errorDescriptions[505] = "The server does not support the HTTP protocol version used in the request.";
    
    errorMap[506] = "Variant Also Negotiates";
    errorDescriptions[506] = "The server has an internal configuration error.";
    
    errorMap[507] = "Insufficient Storage";
    errorDescriptions[507] = "The server is unable to store the representation needed to complete the request.";
    
    errorMap[508] = "Loop Detected";
    errorDescriptions[508] = "The server detected an infinite loop while processing the request.";
    
    errorMap[510] = "Not Extended";
    errorDescriptions[510] = "Further extensions to the request are required for the server to fulfill it.";
    
    errorMap[511] = "Network Authentication Required";
    errorDescriptions[511] = "The client needs to authenticate to gain network access.";
}

std::string HttpError::getErrorMessage(int code) {
    if (errorMap.find(code) != errorMap.end()) {
        return errorMap[code];
    }
    return "Unknown Error";
}

std::string HttpError::getErrorDescription(int code) {
    if (errorDescriptions.find(code) != errorDescriptions.end()) {
        return errorDescriptions[code];
    }
    return "An unknown error occurred.";
}

std::string HttpError::generateHtmlBody(int code, const std::string& message, const std::string& description) {
    std::ostringstream html;
    
    html << "<!DOCTYPE html>\n"
         << "<html lang=\"en\">\n"
         << "<head>\n"
         << "    <meta charset=\"UTF-8\">\n"
         << "    <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n"
         << "    <title>" << code << " " << message << "</title>\n"
         << "    <style>\n"
         << "        * { margin: 0; padding: 0; box-sizing: border-box; }\n"
         << "        body {\n"
         << "            font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, 'Helvetica Neue', Arial, sans-serif;\n"
         << "            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);\n"
         << "            min-height: 100vh;\n"
         << "            display: flex;\n"
         << "            justify-content: center;\n"
         << "            align-items: center;\n"
         << "            padding: 20px;\n"
         << "        }\n"
         << "        .error-container {\n"
         << "            background: white;\n"
         << "            border-radius: 20px;\n"
         << "            box-shadow: 0 20px 60px rgba(0,0,0,0.3);\n"
         << "            padding: 60px;\n"
         << "            max-width: 600px;\n"
         << "            text-align: center;\n"
         << "        }\n"
         << "        .error-code {\n"
         << "            font-size: 120px;\n"
         << "            font-weight: bold;\n"
         << "            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);\n"
         << "            -webkit-background-clip: text;\n"
         << "            -webkit-text-fill-color: transparent;\n"
         << "            margin-bottom: 20px;\n"
         << "        }\n"
         << "        .error-message {\n"
         << "            font-size: 32px;\n"
         << "            color: #333;\n"
         << "            margin-bottom: 20px;\n"
         << "            font-weight: 600;\n"
         << "        }\n"
         << "        .error-description {\n"
         << "            font-size: 18px;\n"
         << "            color: #666;\n"
         << "            line-height: 1.6;\n"
         << "            margin-bottom: 40px;\n"
         << "        }\n"
         << "        .back-button {\n"
         << "            display: inline-block;\n"
         << "            padding: 15px 40px;\n"
         << "            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);\n"
         << "            color: white;\n"
         << "            text-decoration: none;\n"
         << "            border-radius: 50px;\n"
         << "            font-weight: 600;\n"
         << "            transition: transform 0.3s ease;\n"
         << "        }\n"
         << "        .back-button:hover {\n"
         << "            transform: translateY(-3px);\n"
         << "        }\n"
         << "    </style>\n"
         << "</head>\n"
         << "<body>\n"
         << "    <div class=\"error-container\">\n"
         << "        <div class=\"error-code\">" << code << "</div>\n"
         << "        <div class=\"error-message\">" << message << "</div>\n"
         << "        <div class=\"error-description\">" << description << "</div>\n"
         << "        <a href=\"/\" class=\"back-button\">Go Home</a>\n"
         << "    </div>\n"
         << "</body>\n"
         << "</html>";
    
    return html.str();
}

// ============ METODY POMOCNICZE DLA CUSTOM ERROR PAGES ============

bool HttpError::fileExists(const std::string& path) const {
    struct stat buffer;
    return (stat(path.c_str(), &buffer) == 0 && S_ISREG(buffer.st_mode));
}

bool HttpError::isValidPath(const std::string& path) const {
    // Sprawdź czy ścieżka nie zawiera path traversal
    if (path.find("..") != std::string::npos) {
        return false;
    }
    
    // Sprawdź czy plik istnieje i jest czytelny
    return (access(path.c_str(), R_OK) == 0);
}

std::string HttpError::readFileContent(const std::string& path) const {
    std::ifstream file(path.c_str());
    if (!file.is_open()) {
        return "";
    }
    
    std::ostringstream content;
    content << file.rdbuf();
    file.close();
    
    return content.str();
}

// ============ PUBLICZNE METODY ZARZĄDZANIA CUSTOM PAGES ============

void HttpError::setCustomErrorPage(int code, const std::string& filePath) {
    // Walidacja ścieżki
    if (!isValidPath(filePath)) {
        std::cerr << "Warning: Invalid or inaccessible custom error page: " 
                  << filePath << std::endl;
        return;
    }
    
    customErrorPages[code] = filePath;
}

void HttpError::clearCustomErrorPages() {
    customErrorPages.clear();
}

bool HttpError::hasCustomErrorPage(int code) const {
    return customErrorPages.find(code) != customErrorPages.end();
}

std::string HttpError::getCustomErrorPagePath(int code) const {
    std::map<int, std::string>::const_iterator it = customErrorPages.find(code);
    if (it != customErrorPages.end()) {
        return it->second;
    }
    return "";
}

// ============ ZAKTUALIZOWANE METODY GENEROWANIA ODPOWIEDZI ============

std::string HttpError::generateErrorResponse(int code) {
    // 1️⃣ Sprawdź czy istnieje custom error page
    if (hasCustomErrorPage(code)) {
        std::string customPath = getCustomErrorPagePath(code);
        
        // 2️⃣ Sprawdź czy plik nadal istnieje i jest dostępny
        if (fileExists(customPath) && isValidPath(customPath)) {
            std::string customContent = readFileContent(customPath);
            
            // 3️⃣ Jeśli udało się wczytać - użyj custom page
            if (!customContent.empty()) {
                std::string message = getErrorMessage(code);
                
                std::ostringstream response;
                response << "HTTP/1.1 " << code << " " << message << "\r\n"
                         << "Content-Type: text/html; charset=UTF-8\r\n"
                         << "Content-Length: " << customContent.length() << "\r\n"
                         << "Connection: close\r\n"
                         << "\r\n"
                         << customContent;
                
                return response.str();
            } else {
                // ⚠️ Plik pusty lub błąd odczytu - loguj warning
                std::cerr << "Warning: Could not read custom error page: " 
                          << customPath << std::endl;
            }
        } else {
            // ⚠️ Plik już nie istnieje lub stracił uprawnienia
            std::cerr << "Warning: Custom error page no longer accessible: " 
                      << customPath << std::endl;
        }
    }
    
    // 4️⃣ Fallback - użyj domyślnej strony
    std::string message = getErrorMessage(code);
    std::string description = getErrorDescription(code);
    std::string htmlBody = generateHtmlBody(code, message, description);
    
    std::ostringstream response;
    response << "HTTP/1.1 " << code << " " << message << "\r\n"
             << "Content-Type: text/html; charset=UTF-8\r\n"
             << "Content-Length: " << htmlBody.length() << "\r\n"
             << "Connection: close\r\n"
             << "\r\n"
             << htmlBody;
    
    return response.str();
}

std::string HttpError::generateErrorResponse(int code, const std::string& customMessage) {
    // 1️⃣ Sprawdź czy istnieje custom error page (priorytet dla custom page)
    if (hasCustomErrorPage(code)) {
        std::string customPath = getCustomErrorPagePath(code);
        
        if (fileExists(customPath) && isValidPath(customPath)) {
            std::string customContent = readFileContent(customPath);
            
            if (!customContent.empty()) {
                std::string message = getErrorMessage(code);
                
                std::ostringstream response;
                response << "HTTP/1.1 " << code << " " << message << "\r\n"
                         << "Content-Type: text/html; charset=UTF-8\r\n"
                         << "Content-Length: " << customContent.length() << "\r\n"
                         << "Connection: close\r\n"
                         << "\r\n"
                         << customContent;
                
                return response.str();
            }
        }
    }
    
    // 2️⃣ Fallback - użyj domyślnej strony z custom message
    std::string message = getErrorMessage(code);
    std::string htmlBody = generateHtmlBody(code, message, customMessage);
    
    std::ostringstream response;
    response << "HTTP/1.1 " << code << " " << message << "\r\n"
             << "Content-Type: text/html; charset=UTF-8\r\n"
             << "Content-Length: " << htmlBody.length() << "\r\n"
             << "Connection: close\r\n"
             << "\r\n"
             << htmlBody;
    
    return response.str();
}





