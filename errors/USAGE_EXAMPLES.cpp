// PRZYKŁADY UŻYCIA HttpError w różnych częściach serwera
// Ten plik pokazuje jak obsługiwać różne błędy HTTP

#include "errors/error.hpp"
#include <iostream>

void exampleUsage() {
    HttpError errorHandler;
    
    // ============================================
    // PRZYKŁAD 1: Wysyłanie błędu 404 Not Found
    // ============================================
    std::string error404 = errorHandler.generateErrorResponse(404);
    // Teraz wyślij to przez send():
    // send(clientFD, error404.c_str(), error404.size(), 0);
    
    
    // ============================================
    // PRZYKŁAD 2: Wysyłanie błędu 405 Method Not Allowed
    // ============================================
    if (method != "GET" && method != "POST" && method != "DELETE") {
        std::string error405 = errorHandler.generateErrorResponse(405);
        // send(clientFD, error405.c_str(), error405.size(), 0);
    }
    
    
    // ============================================
    // PRZYKŁAD 3: Wysyłanie błędu 500 z custom message
    // ============================================
    std::string error500 = errorHandler.generateErrorResponse(500, 
        "Database connection failed. Please try again later.");
    // send(clientFD, error500.c_str(), error500.size(), 0);
    
    
    // ============================================
    // PRZYKŁAD 4: Sprawdzanie rozmiaru pliku (413)
    // ============================================
    const size_t MAX_FILE_SIZE = 10 * 1024 * 1024; // 10MB
    if (fileSize > MAX_FILE_SIZE) {
        std::string error413 = errorHandler.generateErrorResponse(413,
            "File upload exceeds maximum size of 10MB.");
        // send(clientFD, error413.c_str(), error413.size(), 0);
    }
    
    
    // ============================================
    // PRZYKŁAD 5: Timeout (408 Request Timeout)
    // ============================================
    // if (timeSinceLastRequest > TIMEOUT_SECONDS) {
    //     std::string error408 = errorHandler.generateErrorResponse(408);
    //     send(clientFD, error408.c_str(), error408.size(), 0);
    //     close(clientFD);
    // }
    
    
    // ============================================
    // PRZYKŁAD 6: Pobieranie tylko opisu błędu
    // ============================================
    std::cout << "Error 403: " << errorHandler.getErrorMessage(403) << std::endl;
    // Wypisze: "Error 403: Forbidden"
    
    std::cout << errorHandler.getErrorDescription(403) << std::endl;
    // Wypisze: "The server refuses to authorize the request."
}


// ============================================
// INTEGRACJA Z HTTP PARSER
// ============================================
void httpParserErrorHandling(const std::string& requestStr, int clientFD) {
    HttpError errorHandler;
    
    // Sprawdź czy request ma właściwy format
    if (requestStr.find("HTTP/1.1") == std::string::npos) {
        std::string error400 = errorHandler.generateErrorResponse(400,
            "Invalid HTTP version. Only HTTP/1.1 is supported.");
        send(clientFD, error400.c_str(), error400.size(), 0);
        return;
    }
    
    // Sprawdź długość URI
    size_t uriStart = requestStr.find(" ") + 1;
    size_t uriEnd = requestStr.find(" ", uriStart);
    std::string uri = requestStr.substr(uriStart, uriEnd - uriStart);
    
    if (uri.length() > 2048) {
        std::string error414 = errorHandler.generateErrorResponse(414);
        send(clientFD, error414.c_str(), error414.size(), 0);
        return;
    }
    
    // Sprawdź czy metoda jest obsługiwana
    std::string method = requestStr.substr(0, requestStr.find(" "));
    if (method != "GET" && method != "POST" && method != "DELETE") {
        std::string error501 = errorHandler.generateErrorResponse(501,
            "Method '" + method + "' is not implemented on this server.");
        send(clientFD, error501.c_str(), error501.size(), 0);
        return;
    }
}


// ============================================
// INTEGRACJA Z CGI
// ============================================
void cgiErrorHandling(const std::string& scriptPath, int clientFD) {
    HttpError errorHandler;
    
    // Sprawdź czy skrypt istnieje
    if (access(scriptPath.c_str(), F_OK) != 0) {
        std::string error404 = errorHandler.generateErrorResponse(404,
            "CGI script '" + scriptPath + "' not found.");
        send(clientFD, error404.c_str(), error404.size(), 0);
        return;
    }
    
    // Sprawdź czy skrypt jest wykonywalny
    if (access(scriptPath.c_str(), X_OK) != 0) {
        std::string error403 = errorHandler.generateErrorResponse(403,
            "CGI script exists but is not executable.");
        send(clientFD, error403.c_str(), error403.size(), 0);
        return;
    }
    
    // Jeśli execve() się nie powiedzie
    // W procesie potomnym:
    // execve(interpreterPath, args, environ);
    // Jeśli dojdziemy tutaj, to execve() failed
    std::string error502 = errorHandler.generateErrorResponse(502,
        "Failed to execute CGI script.");
    send(clientFD, error502.c_str(), error502.size(), 0);
    exit(1);
}


// ============================================
// INTEGRACJA Z ROUTER
// ============================================
void routerErrorHandling(const std::string& path, const std::string& method, 
                         const ServerConfig& config, int clientFD) {
    HttpError errorHandler;
    
    // Sprawdź czy route istnieje
    bool routeFound = false;
    // ... logika szukania route ...
    
    if (!routeFound) {
        std::string error404 = errorHandler.generateErrorResponse(404,
            "The requested path '" + path + "' does not exist.");
        send(clientFD, error404.c_str(), error404.size(), 0);
        return;
    }
    
    // Sprawdź czy metoda jest dozwolona dla tego route
    bool methodAllowed = false;
    // ... logika sprawdzania metod ...
    
    if (!methodAllowed) {
        std::string error405 = errorHandler.generateErrorResponse(405,
            "Method '" + method + "' is not allowed for this resource.");
        send(clientFD, error405.c_str(), error405.size(), 0);
        return;
    }
}


// ============================================
// OBSŁUGA BŁĘDÓW W TRY-CATCH
// ============================================
void safeRequestHandling(int clientFD) {
    HttpError errorHandler;
    
    try {
        // Twoja normalna logika serwera
        // ...
    }
    catch (const std::runtime_error& e) {
        std::cerr << "Runtime error: " << e.what() << std::endl;
        std::string error500 = errorHandler.generateErrorResponse(500,
            "A runtime error occurred: " + std::string(e.what()));
        send(clientFD, error500.c_str(), error500.size(), 0);
    }
    catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        std::string error500 = errorHandler.generateErrorResponse(500);
        send(clientFD, error500.c_str(), error500.size(), 0);
    }
    catch (...) {
        std::cerr << "Unknown exception!" << std::endl;
        std::string error500 = errorHandler.generateErrorResponse(500,
            "An unknown error occurred.");
        send(clientFD, error500.c_str(), error500.size(), 0);
    }
}
