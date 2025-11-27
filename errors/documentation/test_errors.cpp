// Test systemu obsługi błędów HTTP
// Kompiluj: g++ -std=c++98 test_errors.cpp errors/error.cpp -o test_errors

#include <iostream>
#include "errors/error.hpp"

void testAllErrorCodes() {
    HttpError errorHandler;
    
    std::cout << "==================================" << std::endl;
    std::cout << "TEST WSZYSTKICH KODÓW BŁĘDÓW" << std::endl;
    std::cout << "==================================" << std::endl;
    
    // Lista kodów do przetestowania
    int testCodes[] = {
        400, 401, 403, 404, 405, 408, 413, 414, 429, 431,
        500, 501, 502, 503, 504, 505
    };
    
    for (size_t i = 0; i < sizeof(testCodes) / sizeof(testCodes[0]); i++) {
        int code = testCodes[i];
        std::cout << "\n--- Kod " << code << " ---" << std::endl;
        std::cout << "Message: " << errorHandler.getErrorMessage(code) << std::endl;
        std::cout << "Description: " << errorHandler.getErrorDescription(code) << std::endl;
    }
}

void testGenerateResponse() {
    HttpError errorHandler;
    
    std::cout << "\n\n==================================" << std::endl;
    std::cout << "TEST GENEROWANIA ODPOWIEDZI HTTP" << std::endl;
    std::cout << "==================================" << std::endl;
    
    // Test 404
    std::cout << "\n--- Test 404 Not Found ---" << std::endl;
    std::string response404 = errorHandler.generateErrorResponse(404);
    std::cout << "Response length: " << response404.length() << " bytes" << std::endl;
    std::cout << "First 200 chars:\n" << response404.substr(0, 200) << "..." << std::endl;
    
    // Test 500 z custom message
    std::cout << "\n--- Test 500 z niestandardowym komunikatem ---" << std::endl;
    std::string response500 = errorHandler.generateErrorResponse(500, 
        "Database connection failed!");
    std::cout << "Response length: " << response500.length() << " bytes" << std::endl;
    std::cout << "First 200 chars:\n" << response500.substr(0, 200) << "..." << std::endl;
}

void testEdgeCases() {
    HttpError errorHandler;
    
    std::cout << "\n\n==================================" << std::endl;
    std::cout << "TEST PRZYPADKÓW BRZEGOWYCH" << std::endl;
    std::cout << "==================================" << std::endl;
    
    // Test nieistniejącego kodu
    std::cout << "\n--- Nieistniejący kod (999) ---" << std::endl;
    std::string message = errorHandler.getErrorMessage(999);
    std::cout << "Message: " << (message.empty() ? "[empty]" : message) << std::endl;
    
    // Test zabawnego kodu 418
    std::cout << "\n--- Zabawny kod 418 I'm a teapot ---" << std::endl;
    std::cout << "Message: " << errorHandler.getErrorMessage(418) << std::endl;
    std::cout << "Description: " << errorHandler.getErrorDescription(418) << std::endl;
}

void performanceTest() {
    HttpError errorHandler;
    
    std::cout << "\n\n==================================" << std::endl;
    std::cout << "TEST WYDAJNOŚCI" << std::endl;
    std::cout << "==================================" << std::endl;
    
    const int iterations = 1000;
    std::cout << "Generowanie " << iterations << " odpowiedzi błędów..." << std::endl;
    
    clock_t start = clock();
    for (int i = 0; i < iterations; i++) {
        errorHandler.generateErrorResponse(404);
    }
    clock_t end = clock();
    
    double elapsed = double(end - start) / CLOCKS_PER_SEC * 1000.0;
    std::cout << "Czas: " << elapsed << " ms" << std::endl;
    std::cout << "Średnio: " << elapsed / iterations << " ms na błąd" << std::endl;
}

int main() {
    std::cout << "\n";
    std::cout << "╔═══════════════════════════════════════════╗" << std::endl;
    std::cout << "║  TEST SYSTEMU OBSŁUGI BŁĘDÓW HTTP        ║" << std::endl;
    std::cout << "╚═══════════════════════════════════════════╝" << std::endl;
    
    testAllErrorCodes();
    testGenerateResponse();
    testEdgeCases();
    performanceTest();
    
    std::cout << "\n\n✅ Wszystkie testy zakończone!" << std::endl;
    std::cout << "============================================\n" << std::endl;
    
    return 0;
}
