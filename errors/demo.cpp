// DEMO: Prosty przykład użycia systemu błędów
// Kompiluj: g++ -std=c++98 demo.cpp errors/error.cpp -o demo

#include <iostream>
#include "errors/error.hpp"

void demo404() {
    std::cout << "\n╔══════════════════════════════════════════╗" << std::endl;
    std::cout << "║  DEMO: Błąd 404 Not Found                ║" << std::endl;
    std::cout << "╚══════════════════════════════════════════╝\n" << std::endl;
    
    HttpError errorHandler;
    std::string response = errorHandler.generateErrorResponse(404);
    
    std::cout << "Wygenerowana odpowiedź HTTP:\n" << std::endl;
    std::cout << response.substr(0, 500) << "..." << std::endl;
    std::cout << "\nDługość: " << response.length() << " bajtów" << std::endl;
}

void demo500WithCustomMessage() {
    std::cout << "\n╔══════════════════════════════════════════╗" << std::endl;
    std::cout << "║  DEMO: Błąd 500 z custom message         ║" << std::endl;
    std::cout << "╚══════════════════════════════════════════╝\n" << std::endl;
    
    HttpError errorHandler;
    std::string response = errorHandler.generateErrorResponse(500, 
        "Database connection failed. Please try again later.");
    
    std::cout << "Wygenerowana odpowiedź HTTP z niestandardowym komunikatem:\n" << std::endl;
    std::cout << response.substr(0, 500) << "..." << std::endl;
}

void demoGetDescriptions() {
    std::cout << "\n╔══════════════════════════════════════════╗" << std::endl;
    std::cout << "║  DEMO: Pobieranie opisów błędów          ║" << std::endl;
    std::cout << "╚══════════════════════════════════════════╝\n" << std::endl;
    
    HttpError errorHandler;
    
    int codes[] = {400, 403, 404, 405, 413, 500, 502, 503};
    
    for (size_t i = 0; i < sizeof(codes) / sizeof(codes[0]); i++) {
        int code = codes[i];
        std::cout << code << " " << errorHandler.getErrorMessage(code) << std::endl;
        std::cout << "   → " << errorHandler.getErrorDescription(code) << "\n" << std::endl;
    }
}

void demoTeapot() {
    std::cout << "\n╔══════════════════════════════════════════╗" << std::endl;
    std::cout << "║  DEMO: Zabawny kod 418 I'm a teapot 🫖   ║" << std::endl;
    std::cout << "╚══════════════════════════════════════════╝\n" << std::endl;
    
    HttpError errorHandler;
    
    std::cout << "Kod: 418" << std::endl;
    std::cout << "Nazwa: " << errorHandler.getErrorMessage(418) << std::endl;
    std::cout << "Opis: " << errorHandler.getErrorDescription(418) << std::endl;
    std::cout << "\nTen kod pochodzi z RFC 2324 (Hyper Text Coffee Pot Control Protocol)" << std::endl;
    std::cout << "i jest żartem April Fools! 😄\n" << std::endl;
}

void demoRealWorldScenario() {
    std::cout << "\n╔══════════════════════════════════════════╗" << std::endl;
    std::cout << "║  DEMO: Rzeczywisty scenariusz użycia     ║" << std::endl;
    std::cout << "╚══════════════════════════════════════════╝\n" << std::endl;
    
    // Symuluj walidację requesta
    std::string requestStr = "";  // Pusty request od klienta
    
    std::cout << "Klient wysłał pusty request..." << std::endl;
    
    if (requestStr.empty()) {
        std::cout << "❌ Wykryto pusty request!" << std::endl;
        
        HttpError errorHandler;
        std::string errorResponse = errorHandler.generateErrorResponse(400,
            "The request is empty or does not contain valid HTTP headers.");
        
        std::cout << "✅ Wygenerowano błąd 400 Bad Request" << std::endl;
        std::cout << "📤 Wysyłam do klienta przez send()..." << std::endl;
        
        // W prawdziwym kodzie:
        // send(clientFD, errorResponse.c_str(), errorResponse.size(), 0);
        
        std::cout << "✓ Klient otrzyma piękną stronę błędu HTML!\n" << std::endl;
    }
}

void demoAllCategories() {
    std::cout << "\n╔══════════════════════════════════════════╗" << std::endl;
    std::cout << "║  DEMO: Kategorie błędów HTTP             ║" << std::endl;
    std::cout << "╚══════════════════════════════════════════╝\n" << std::endl;
    
    HttpError errorHandler;
    
    std::cout << "🔴 4xx - BŁĘDY KLIENTA (Client Errors):" << std::endl;
    std::cout << "   400 " << errorHandler.getErrorMessage(400) << std::endl;
    std::cout << "   401 " << errorHandler.getErrorMessage(401) << std::endl;
    std::cout << "   403 " << errorHandler.getErrorMessage(403) << std::endl;
    std::cout << "   404 " << errorHandler.getErrorMessage(404) << std::endl;
    std::cout << "   405 " << errorHandler.getErrorMessage(405) << std::endl;
    std::cout << "   413 " << errorHandler.getErrorMessage(413) << std::endl;
    std::cout << "   414 " << errorHandler.getErrorMessage(414) << std::endl;
    std::cout << "   429 " << errorHandler.getErrorMessage(429) << std::endl;
    
    std::cout << "\n🟠 5xx - BŁĘDY SERWERA (Server Errors):" << std::endl;
    std::cout << "   500 " << errorHandler.getErrorMessage(500) << std::endl;
    std::cout << "   501 " << errorHandler.getErrorMessage(501) << std::endl;
    std::cout << "   502 " << errorHandler.getErrorMessage(502) << std::endl;
    std::cout << "   503 " << errorHandler.getErrorMessage(503) << std::endl;
    std::cout << "   504 " << errorHandler.getErrorMessage(504) << std::endl;
    std::cout << "   505 " << errorHandler.getErrorMessage(505) << std::endl;
}

int main() {
    std::cout << "\n";
    std::cout << "╔════════════════════════════════════════════════════╗" << std::endl;
    std::cout << "║                                                    ║" << std::endl;
    std::cout << "║      HTTP ERROR HANDLING SYSTEM - DEMO             ║" << std::endl;
    std::cout << "║      Stworzone dla projektu webserv               ║" << std::endl;
    std::cout << "║                                                    ║" << std::endl;
    std::cout << "╚════════════════════════════════════════════════════╝" << std::endl;
    
    demo404();
    demo500WithCustomMessage();
    demoGetDescriptions();
    demoTeapot();
    demoRealWorldScenario();
    demoAllCategories();
    
    std::cout << "\n╔════════════════════════════════════════════════════╗" << std::endl;
    std::cout << "║  ✅ DEMO ZAKOŃCZONE POMYŚLNIE!                     ║" << std::endl;
    std::cout << "╚════════════════════════════════════════════════════╝\n" << std::endl;
    
    std::cout << "📚 Sprawdź dokumentację w:" << std::endl;
    std::cout << "   • errors/README.md" << std::endl;
    std::cout << "   • errors/USAGE_EXAMPLES.cpp" << std::endl;
    std::cout << "   • errors/INTEGRATION_GUIDE.cpp" << std::endl;
    std::cout << "   • errors/QUICK_REFERENCE.txt" << std::endl;
    
    std::cout << "\n🚀 Gotowe do użycia w webserverze!\n" << std::endl;
    
    return 0;
}
