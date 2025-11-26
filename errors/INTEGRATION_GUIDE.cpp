// INSTRUKCJA: JAK ZINTEGROWAĆ SYSTEM BŁĘDÓW Z POLL.CPP
// ========================================================

/*
 * KROK 1: Dodaj include na początku poll.cpp
 */

#include "CoreEngine.hpp"
#include "../http/Http.hpp"
#include "../errors/error.hpp"  // ← DODAJ TĘ LINIJKĘ


/*
 * KROK 2: Zmień funkcję recivNClose() w poll.cpp
 */

void CoreEngine::recivNClose(size_t el)
{
   // NOWA WERSJA Z OBSŁUGĄ BŁĘDÓW:
   
   // Wyczyść buffer przed recv (zapobiega śmieciom w pamięci)
   memset(buffer, 0, sizeof(buffer));
   
   byteRecived = recv(pollFDs[el].fd, buffer, sizeof(buffer) - 1, 0);
   
   // BŁĄD recv() → Wyślij 500 Internal Server Error
   if (byteRecived == -1)
   {
      std::cerr << "recv() failed: " << strerror(errno) << std::endl;
      
      HttpError errorHandler;
      std::string errorResponse = errorHandler.generateErrorResponse(500);
      send(pollFDs[el].fd, errorResponse.c_str(), errorResponse.size(), 0);
      close(pollFDs[el].fd);
      return;
   }
   
   // Klient zamknął połączenie (EOF)
   else if (byteRecived == 0)
   {
      close(pollFDs[el].fd);
      for (size_t i = el; i < pollFDsNum; i++)
      {
         if (el == (pollFDsNum - 1))
            break;
         pollFDs[i] = pollFDs[i + 1];
      }
      pollFDs = (pollfd *)realloc(pollFDs, (pollFDsNum - 1) * sizeof(pollfd));
      pollFDsNum--;
   }
   
   // Odebrano dane
   else
   {
      buffer[byteRecived] = '\0';  // Null-terminate string
      std::string requestStr(buffer);
      
      // WALIDACJA 1: Sprawdź czy request jest pusty → 400 Bad Request
      if (requestStr.empty() || requestStr.find("HTTP") == std::string::npos)
      {
         std::cout << "Empty or invalid HTTP request!" << std::endl;
         
         HttpError errorHandler;
         std::string errorResponse = errorHandler.generateErrorResponse(400, 
            "Empty or malformed HTTP request.");
         send(pollFDs[el].fd, errorResponse.c_str(), errorResponse.size(), 0);
         close(pollFDs[el].fd);
         return;
      }
      
      // WALIDACJA 2: Sprawdź czy buffer był za mały → 413 Content Too Large
      if (byteRecived == (int)(sizeof(buffer) - 1))
      {
         std::cout << "Request too large! Buffer full." << std::endl;
         
         HttpError errorHandler;
         std::string errorResponse = errorHandler.generateErrorResponse(413,
            "Request payload exceeds maximum buffer size.");
         send(pollFDs[el].fd, errorResponse.c_str(), errorResponse.size(), 0);
         close(pollFDs[el].fd);
         return;
      }
      
      std::cout << "--->buffer: " << buffer << std::endl;
      pollFDs[el].events = POLLOUT;
   }
}


/*
 * KROK 3: Zmień funkcję sendToClient() w poll.cpp
 */

void CoreEngine::sendToClient(size_t el)
{
   HttpError errorHandler;
   
   try
   {
      std::string requestStr(buffer);
      Http response(requestStr, serversCfg[0]);
      std::string responseStr = response.responseBuilder();
      
      int byteSend = send(pollFDs[el].fd, responseStr.c_str(), responseStr.size(), 0);
      
      if (byteSend == -1)
      {
         std::cerr << "send() failed: " << strerror(errno) << std::endl;
         
         // Wyślij 500 Internal Server Error
         std::string errorResponse = errorHandler.generateErrorResponse(500);
         send(pollFDs[el].fd, errorResponse.c_str(), errorResponse.size(), 0);
         return;
      }
      
      std::cout << "Packet sent successfully!" << std::endl;
   }
   catch (const std::exception& e)
   {
      // Jeśli cokolwiek pójdzie nie tak (parser, response builder, itp.)
      std::cerr << "Exception in sendToClient: " << e.what() << std::endl;
      
      std::string errorResponse = errorHandler.generateErrorResponse(500,
         "Internal error while processing request.");
      send(pollFDs[el].fd, errorResponse.c_str(), errorResponse.size(), 0);
   }
   
   pollFDs[el].events = POLLIN;
}


/*
 * KROK 4: (OPCJONALNIE) Zwiększ rozmiar buffera w CoreEngine.hpp
 */

// W pliku CoreEngine.hpp zmień:
// char buffer[1024];  // ← STARY (za mały)
// na:
// char buffer[8192];  // ← NOWY (zalecany dla HTTP)


/*
 * KROK 5: Dodaj obsługę błędów w innych miejscach
 */

// Przykład: W routerze sprawdź czy route istnieje
void checkRoute(const std::string& path, int clientFD) {
    bool routeExists = false;
    // ... logika sprawdzania ...
    
    if (!routeExists) {
        HttpError errorHandler;
        std::string errorResponse = errorHandler.generateErrorResponse(404,
            "The requested path '" + path + "' does not exist.");
        send(clientFD, errorResponse.c_str(), errorResponse.size(), 0);
        return;
    }
}

// Przykład: Sprawdź metodę HTTP
void checkMethod(const std::string& method, int clientFD) {
    if (method != "GET" && method != "POST" && method != "DELETE") {
        HttpError errorHandler;
        std::string errorResponse = errorHandler.generateErrorResponse(405,
            "Method '" + method + "' is not allowed.");
        send(clientFD, errorResponse.c_str(), errorResponse.size(), 0);
        return;
    }
}

// Przykład: Sprawdź długość URI
void checkUriLength(const std::string& uri, int clientFD) {
    const size_t MAX_URI_LENGTH = 2048;
    
    if (uri.length() > MAX_URI_LENGTH) {
        HttpError errorHandler;
        std::string errorResponse = errorHandler.generateErrorResponse(414);
        send(clientFD, errorResponse.c_str(), errorResponse.size(), 0);
        return;
    }
}


/*
 * PODSUMOWANIE ZMIAN:
 * ====================
 * 
 * 1. ✅ Dodano #include "errors/error.hpp"
 * 2. ✅ Dodano memset() przed recv() (zapobiega śmieciom)
 * 3. ✅ Dodano obsługę błędu recv() → 500
 * 4. ✅ Dodano walidację pustego requesta → 400
 * 5. ✅ Dodano walidację rozmiaru buffera → 413
 * 6. ✅ Dodano try-catch w sendToClient() → 500
 * 7. ✅ Zwiększono buffer do 8192 bajtów (opcjonalnie)
 * 
 * TESTOWANIE:
 * ===========
 * 
 * Test 1 - Pusty request:
 *   echo "" | nc localhost 8080
 *   Oczekiwany wynik: 400 Bad Request
 * 
 * Test 2 - Zbyt duży request:
 *   curl -X POST http://localhost:8080/ -d "$(python -c 'print("A"*10000)')"
 *   Oczekiwany wynik: 413 Content Too Large
 * 
 * Test 3 - Normalna strona:
 *   curl http://localhost:8080/
 *   Oczekiwany wynik: 200 OK (lub twoja normalna odpowiedź)
 * 
 * Test 4 - Nieistniejąca strona:
 *   curl http://localhost:8080/nieistniejaca
 *   Oczekiwany wynik: 404 Not Found (gdy dodasz to do routera)
 */
