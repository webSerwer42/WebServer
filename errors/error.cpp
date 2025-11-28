/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   error.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: agorski <agorski@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/26 14:37:10 by agorski           #+#    #+#             */
/*   Updated: 2025/11/27 17:29:00 by agorski          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "error.hpp"
#include <fstream>
#include <sys/stat.h>
#include <unistd.h>
#include <cstdlib>
#include <climits>
#include <cstring>

HttpError::HttpError() {
    initializeErrorMaps();
}

void HttpError::initializeErrorMaps() {
    // ============ 4xx CLIENT ERRORS ============
    
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
    
    // ============ 5xx SERVER ERRORS ============
    
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
         << "<html>\n"
         << "<head>\n"
         << "<meta charset=\"UTF-8\">\n"
         << "<title>" << code << " " << message << "</title>\n"
         << "</head>\n"
         << "<body>\n"
         << "<h1>" << code << " " << message << "</h1>\n"
         << "<p>" << description << "</p>\n"
         << "</body>\n"
         << "</html>";

    return html.str();
}

// ============ METODY POMOCNICZE DLA DEFAULT ERROR PAGES ============

// Helper function to decode a hex digit
static int hexToInt(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
}

// URL decode a string to handle encoded path traversal sequences
static std::string urlDecode(const std::string& str) {
    std::string result;
    result.reserve(str.length());
    
    for (size_t i = 0; i < str.length(); ++i) {
        if (str[i] == '%' && i + 2 < str.length()) {
            int high = hexToInt(str[i + 1]);
            int low = hexToInt(str[i + 2]);
            if (high >= 0 && low >= 0) {
                result += static_cast<char>((high << 4) | low);
                i += 2;
                continue;
            }
        }
        result += str[i];
    }
    return result;
}

bool HttpError::fileExists(const std::string& path) const {
    struct stat buffer;
    return (stat(path.c_str(), &buffer) == 0 && S_ISREG(buffer.st_mode));
}

bool HttpError::isValidPath(const std::string& path) const {
    // First, decode URL-encoded characters to catch encoded ".." sequences
    std::string decodedPath = urlDecode(path);
    
    // Check for ".." in the decoded path as a quick filter
    if (decodedPath.find("..") != std::string::npos) {
        return false;
    }
    
    // Use realpath() to resolve the absolute, canonical path
    // This handles symlinks, "..", "." and normalizes the path
    char resolvedPath[PATH_MAX];
    if (realpath(decodedPath.c_str(), resolvedPath) == NULL) {
        // File doesn't exist or path cannot be resolved
        return false;
    }
    
    // Get current working directory as the base allowed directory
    char cwdBuffer[PATH_MAX];
    if (getcwd(cwdBuffer, PATH_MAX) == NULL) {
        return false;
    }
    
    // Ensure the resolved path starts with the current working directory
    // This prevents path traversal outside the server's root directory
    std::string resolvedStr(resolvedPath);
    std::string cwdStr(cwdBuffer);
    
    if (resolvedStr.find(cwdStr) != 0) {
        // The resolved path is outside the allowed directory
        return false;
    }
    
    // Check if file exists and is readable
    return (access(resolvedPath, R_OK) == 0);
}

std::string HttpError::readFileContent(const std::string& path) const {
    std::ifstream file(path.c_str());
    if (!file.is_open()) {
        return "";
    }

    std::ostringstream content;
    content << file.rdbuf(); // wczytaj cały plik do strumienia
    file.close();

    return content.str();
}

// ============ PUBLICZNE METODY ZARZĄDZANIA CUSTOM PAGES ============

void HttpError::setDefaultErrorPage(int code, const std::string& filePath) {
    // Walidacja ścieżki
    if (!isValidPath(filePath)) {
        std::cerr << "Warning: Invalid or inaccessible custom error page: " 
                  << filePath << std::endl;
        return;
    }
    defaultErrorPages[code] = filePath;
}

void HttpError::clearDefaultErrorPages() {
    defaultErrorPages.clear();
}

bool HttpError::hasDefaultErrorPage(int code) const {
    return defaultErrorPages.find(code) != defaultErrorPages.end();
}

std::string HttpError::getDefaultErrorPagePath(int code) const {
    std::map<int, std::string>::const_iterator it = defaultErrorPages.find(code);
    if (it != defaultErrorPages.end()) {
        return it->second;
    }
    return "";
}

// ============ ZAKTUALIZOWANE METODY GENEROWANIA ODPOWIEDZI ============

std::string HttpError::generateErrorResponse(int code) {
    // Sprawdź czy istnieje default error page
    if (hasDefaultErrorPage(code)) {
        std::string defaultPath = getDefaultErrorPagePath(code);

        // Sprawdź czy plik nadal istnieje i jest dostępny
        if (fileExists(defaultPath) && isValidPath(defaultPath)) {
            std::string defaultContent = readFileContent(defaultPath);

            // Jeśli udało się wczytać - użyj default page
            if (!defaultContent.empty()) {
                std::string message = getErrorMessage(code);

                std::ostringstream response;
                response << "HTTP/1.1 " << code << " " << message << "\r\n"
                         << "Content-Type: text/html; charset=UTF-8\r\n"
                         << "Content-Length: " << defaultContent.length() << "\r\n"
                         << "Connection: close\r\n"
                         << "\r\n"
                         << defaultContent;

                return response.str();
            } else {
                // Plik pusty lub błąd odczytu - loguj warning
                std::cerr << "Warning: Could not read default error page: " 
                          << defaultPath << std::endl;
            }
        } else {
            // Plik już nie istnieje lub stracił uprawnienia
            std::cerr << "Warning: Custom error page no longer accessible: " 
                      << defaultPath << std::endl;
            code = 500; // Zmień kod na 500 Internal Server Error
        }
    }
    
    // Fallback - użyj domyślnej strony
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
    
        // Sprawdź czy istnieje default error page
    if (hasDefaultErrorPage(code)) {
        std::string defaultPath = getDefaultErrorPagePath(code);

        // Sprawdź czy plik nadal istnieje i jest dostępny
        if (fileExists(defaultPath) && isValidPath(defaultPath)) {
            std::string defaultContent = readFileContent(defaultPath);

            // Jeśli udało się wczytać - użyj default page
            if (!defaultContent.empty()) {
                std::string message = getErrorMessage(code);

                std::ostringstream response;
                response << "HTTP/1.1 " << code << " " << message << "\r\n"
                         << "Content-Type: text/html; charset=UTF-8\r\n"
                         << "Content-Length: " << defaultContent.length() << "\r\n"
                         << "Connection: close\r\n"
                         << "\r\n"
                         << defaultContent;

                return response.str();
            } else {
                // Plik pusty lub błąd odczytu - loguj warning
                std::cerr << "Warning: Could not read default error page: " 
                          << defaultPath << std::endl;
            }
        } else {
            // Plik już nie istnieje lub stracił uprawnienia
            std::cerr << "Warning: Custom error page no longer accessible: " 
                      << defaultPath << std::endl;
            code = 500; // Zmień kod na 500 Internal Server Error
        }
    }
    
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





