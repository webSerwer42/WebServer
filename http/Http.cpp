#include "Http.hpp"
#include "stringUtils.hpp"
#include <iostream>

// ============ PARSING FUNCTIONS ============

void Http::parseRequest(const std::string &rawRequest) {
    if (rawRequest.empty()) {
        std::cerr << "Error: Empty HTTP request" << std::endl;
        return;
    }
    
    // Find the first non-whitespace character
    size_t reqLineStart = rawRequest.find_first_not_of(" \t\r\n");
    if (reqLineStart == std::string::npos) {
        std::cerr << "Error: Request contains only whitespace" << std::endl;
        return;
    }
    
    // Find the end of the request line
    size_t firstLineEnd = rawRequest.find("\r\n", reqLineStart);
    size_t firstLineEndLen = 2;
    
    if (firstLineEnd == std::string::npos) {
        firstLineEnd = rawRequest.find("\n", reqLineStart);
        if (firstLineEnd == std::string::npos) {
            std::cerr << "Error: Could not find end of request line" << std::endl;
            return;
        }
        firstLineEndLen = 1;
    }
    
    // Extract and parse request line
    std::string firstLineRaw = rawRequest.substr(reqLineStart, firstLineEnd - reqLineStart);
    parseRequestLine(trim(firstLineRaw));
    
    if (requestMethod.empty()) {
        std::cerr << "Error: Request line parsing failed" << std::endl;
        return;
    }
    
    // Parse headers
    size_t headerStart = firstLineEnd + firstLineEndLen;
    std::vector<std::string> headerLines;
    size_t currentPos = headerStart;
    
    while (currentPos < rawRequest.length()) {
        size_t nextLineEnd = rawRequest.find("\r\n", currentPos);
        size_t nextLineEndLen = 2;
        
        if (nextLineEnd == std::string::npos) {
            nextLineEnd = rawRequest.find("\n", currentPos);
            if (nextLineEnd != std::string::npos) {
                nextLineEndLen = 1;
            } else {
                break;
            }
        }
        
        std::string line = trim(rawRequest.substr(currentPos, nextLineEnd - currentPos));
        
        if (line.empty()) {
            // Empty line marks end of headers
            break;
        }
        
        headerLines.push_back(line);
        currentPos = nextLineEnd + nextLineEndLen;
    }
    
    // Parse each header line
    for (size_t i = 0; i < headerLines.size(); ++i) {
        parseHeaderLine(headerLines[i]);
    }
    
    // Parse query string from URI
    parseQueryString();
}

void Http::parseRequestLine(const std::string &line) {
    std::vector<std::string> parts = split(line, ' ');
    
    // Filter out empty strings
    std::vector<std::string> validParts;
    for (size_t i = 0; i < parts.size(); ++i) {
        if (!parts[i].empty()) {
            validParts.push_back(parts[i]);
        }
    }
    
    if (validParts.size() != 3) {
        std::cerr << "Error: Invalid request line format. Expected 3 parts, got " 
                  << validParts.size() << std::endl;
        return;
    }
    
    requestMethod = validParts[0];
    requestPath = validParts[1];
    requestHttpVersion = validParts[2];
}

void Http::parseHeaderLine(const std::string &line) {
    size_t colonPos = line.find(':');
    
    if (colonPos != std::string::npos) {
        std::string key = trim(line.substr(0, colonPos));
        std::string value = trim(line.substr(colonPos + 1));
        requestHeaders[toLower(key)] = value;
    }
}

void Http::parseQueryString() {
    size_t queryPos = requestPath.find('?');
    
    if (queryPos != std::string::npos) {
        std::string queryString = requestPath.substr(queryPos + 1);
        std::vector<std::string> params = split(queryString, '&');
        
        for (size_t i = 0; i < params.size(); ++i) {
            const std::string& param = params[i];
            size_t equalPos = param.find('=');
            
            if (equalPos != std::string::npos) {
                std::string key = param.substr(0, equalPos);
                std::string value = param.substr(equalPos + 1);
                requestHeaders[key] = value;  // Store query params in headers map for now
            }
        }
        
        // Remove query string from path
        requestPath = requestPath.substr(0, queryPos);
    }
}

// ============ RESPONSE FUNCTIONS ============

void Http::cgiResponse() {
    // TODO: Implement CGI response
    responseStatusCode = "200 OK";
    responseBody = "<html><body><h1>CGI Response</h1></body></html>";
}

void Http::getResponse() {
    // TODO: Implement GET response
    responseStatusCode = "200 OK";
    responseBody = "<html><body><h1>GET Response</h1></body></html>";
}

void Http::postResponse() {
    // TODO: Implement POST response
    responseStatusCode = "200 OK";
    responseBody = "<html><body><h1>POST Response</h1></body></html>";
}

void Http::deleteResponse() {
    // TODO: Implement DELETE response
    responseStatusCode = "200 OK";
    responseBody = "<html><body><h1>DELETE Response</h1></body></html>";
}

// ============ PUBLIC METHODS ============

std::string Http::responseBuilder() {
    // Determine which response to generate based on method
    if (requestMethod == "GET") {
        getResponse();
    } else if (requestMethod == "POST") {
        postResponse();
    } else if (requestMethod == "DELETE") {
        deleteResponse();
    } else {
        responseStatusCode = "405 Method Not Allowed";
        responseBody = "<html><body><h1>405 Method Not Allowed</h1></body></html>";
    }
    
    // Build HTTP response
    std::ostringstream response;
    response << "HTTP/1.1 " << responseStatusCode << "\r\n";
    response << "Content-Type: text/html\r\n";
    response << "Content-Length: " << responseBody.length() << "\r\n";
    response << "Connection: close\r\n";
    response << "\r\n";
    response << responseBody;
    
    return response.str();
}
