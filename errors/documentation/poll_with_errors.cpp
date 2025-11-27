// Copyright [2025] <Piotr Ruszkiewicz> <pruszkie@student.42warsaw.pl>
// Ulepszona wersja z obsługą błędów HTTP
// Copyright [2025] <Antek Górski> <agorski@student.42warsaw.pl>

#include "CoreEngine.hpp"
#include "../http/Http.hpp"
#include "../errors/error.hpp"

void CoreEngine::setConnection(size_t i)
{
   socklen_t addrLen = sizeof(sockaddr_storage);
   int clientFD = accept(pollFDs[i].fd, (struct sockaddr *)&clientSockaddr, &addrLen);
   if (clientFD == -1)
   {
      std::cerr << "accept() failed: " << strerror(errno) << std::endl;
      exit(1);
   }
   pollFDs = (pollfd *)realloc(pollFDs, (pollFDsNum + 1) * sizeof(pollfd));
   pollFDs[pollFDsNum].fd = clientFD;
   pollFDs[pollFDsNum].revents = 0;

   isClientFD[pollFDsNum] = true;
   pollFDs[pollFDsNum].events = POLLIN;
   pollFDsNum++;
   std::cout << "socket: " << pollFDs[i].fd << " ready to connect" << std::endl;
}

void CoreEngine::recivNClose(size_t el)
{
   // ========== WALIDACJA 1: Sprawdź buffer overflow (413) ==========
   memset(buffer, 0, sizeof(buffer));  // Wyczyść buffer przed recv
   
   byteRecived = recv(pollFDs[el].fd, buffer, sizeof(buffer) - 1, 0);
   
   if (byteRecived == -1)
   {
      std::cerr << "recv() failed: " << strerror(errno) << std::endl;
      // Wyślij 500 Internal Server Error
      HttpError errorHandler;
      std::string errorResponse = errorHandler.generateErrorResponse(500);
      send(pollFDs[el].fd, errorResponse.c_str(), errorResponse.size(), 0);
      close(pollFDs[el].fd);
      return;
   }
   // Zamknięcie połączenia przez klienta (EOF)
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
   else
   {
      buffer[byteRecived] = '\0';  // Null-terminate the string
      
      // ========== WALIDACJA 2: Sprawdź czy request jest pusty (400) ==========
      std::string requestStr(buffer);
      if (requestStr.empty() || requestStr.find("HTTP") == std::string::npos)
      {
         std::cout << "Empty or invalid HTTP request detected!" << std::endl;
         HttpError errorHandler;
         std::string errorResponse = errorHandler.generateCostumErrorResponse(400, 
            "The request is empty or does not contain valid HTTP headers.");
         send(pollFDs[el].fd, errorResponse.c_str(), errorResponse.size(), 0);
         close(pollFDs[el].fd);
         return;
      }
      
      // ========== WALIDACJA 3: Sprawdź czy buffer był za mały (413) ==========
      if (byteRecived == (int)(sizeof(buffer) - 1))
      {
         std::cout << "Request too large! Buffer full." << std::endl;
         HttpError errorHandler;
         std::string errorResponse = errorHandler.generateCostumErrorResponse(413,
            "The request payload exceeds the maximum buffer size of 1024 bytes.");
         send(pollFDs[el].fd, errorResponse.c_str(), errorResponse.size(), 0);
         close(pollFDs[el].fd);
         return;
      }
      
      std::cout << "--->buffer: " << buffer << std::endl;
      pollFDs[el].events = POLLOUT;
   }
}

void CoreEngine::sendToClient(size_t el)
{
   std::string requestStr(buffer);
   
   // ========== OBSŁUGA BŁĘDÓW Z TRY-CATCH ==========
   try
   {
      Http response(requestStr, serversCfg[0]); // temp serverCfg
      std::string responseStr = response.responseBuilder();
      
      int byteSend = send(pollFDs[el].fd, responseStr.c_str(), responseStr.size(), 0);
      
      if (byteSend == -1)
      {
         std::cerr << "send() failed: " << strerror(errno) << std::endl;
         
         // Wyślij 500 Internal Server Error
         HttpError errorHandler;
         std::string errorResponse = errorHandler.generateErrorResponse(500);
         send(pollFDs[el].fd, errorResponse.c_str(), errorResponse.size(), 0);
         return;
      }
      
      std::cout << "Packet sent successfully!" << std::endl;
   }
   catch (const std::exception& e)
   {
      // Jeśli cokolwiek pójdzie nie tak podczas parsowania lub generowania response
      std::cerr << "Exception in sendToClient: " << e.what() << std::endl;
      
      HttpError errorHandler;
      std::string errorResponse = errorHandler.generateCostumErrorResponse(500,
         "An internal error occurred while processing your request.");
      send(pollFDs[el].fd, errorResponse.c_str(), errorResponse.size(), 0);
   }
   
   pollFDs[el].events = POLLIN;
}
