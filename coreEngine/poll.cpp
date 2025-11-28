// Copyright [2025] <Piotr Ruszkiewicz> <pruszkie@student.42warsaw.pl>

#include "CoreEngine.hpp"
#include "../http/Http.hpp"
#include "../errors/error.hpp"

void CoreEngine::closeClientConnection(size_t el)
{
   close(pollFDs[el].fd);
   
   // Wyczyść mapowanie dla tego klienta
   clientToServer.erase(el);
   
   // Przesuń elementy w pollFDs i zaktualizuj indeksy w mapowaniach
   for (size_t i = el; i < pollFDsNum - 1; i++)
   {
      pollFDs[i] = pollFDs[i + 1];
      
      // Zaktualizuj mapowania (indeksy się przesunęły!)
      if (clientToServer.find(i + 1) != clientToServer.end())
      {
         clientToServer[i] = clientToServer[i + 1];
         clientToServer.erase(i + 1);
      }
   }
   
   // Zmniejsz rozmiar tablicy pollFDs
   pollFDs = (pollfd *)realloc(pollFDs, (pollFDsNum - 1) * sizeof(pollfd));
   pollFDsNum--;
}

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
   clientToServer[pollFDsNum] = serverFDtoIndex[pollFDs[i].fd]; // zapisz który serwer przyjął połączenie
   pollFDs[pollFDsNum].events = POLLIN;
   pollFDsNum++;
   std::cout << "socket: " << pollFDs[i].fd << " ready to connect" << std::endl;
}

void CoreEngine::recivNClose(size_t el)
{
   // recived date is send to buffer, for now its strign
   byteRecived = recv(pollFDs[el].fd, buffer, 1024, 0);
   buffer[byteRecived] = '\0';
   if (byteRecived == -1)
   {
      std::cerr << "recv() failed: " << strerror(errno) << std::endl;
      // Wyślij 500 Internal Server Error
      HttpError errorHandler;
      std::string errorResponse = errorHandler.generateErrorResponse(500);
      send(pollFDs[el].fd, errorResponse.c_str(), errorResponse.size(), 0);
      
      closeClientConnection(el);
      return;
   }
   // this is closing socket logic, when send EOF by client EOF
   else if (byteRecived == 0)
   {
      closeClientConnection(el);
   }
   else
   {

      //czy request jest pusty (400)
      std::string requestStr(client.buffer);
      if (requestStr.empty() || requestStr.find("HTTP") == std::string::npos)
      {
         std::cout << "Empty or invalid HTTP request detected!" << std::endl;
         HttpError errorHandler;
         std::string errorResponse = errorHandler.generateErrorResponse(400, 
            "The request is empty or does not contain valid HTTP headers.");
         send(pollFDs[el].fd, errorResponse.c_str(), errorResponse.size(), 0);
         
         closeClientConnection(el);
         return;
      }
      
      //czy buffer jest za mały (413)
      if (client.byteRecived == (int)(sizeof(client.buffer) - 1))
      {
         std::cout << "Request too large! Buffer full." << std::endl;
         HttpError errorHandler;
         std::string errorResponse = errorHandler.generateErrorResponse(413,
            "The request payload exceeds the maximum buffer size of 1024 bytes.");
         send(pollFDs[el].fd, errorResponse.c_str(), errorResponse.size(), 0);
         
         closeClientConnection(el);
         return;
      }

      // response to HTTP reqest
      std::cout << "--->buffer: " << buffer << std::endl; // print buffer
      pollFDs[el].events = POLLOUT;
   }
}

void CoreEngine::sendToClient(size_t el)
{

try
{
   client &client = this->getClientByFD(pollFDs[el].fd);
   std::string requestStr(client.buffer);
   size_t serverIndex = clientToServer[el]; // pobierz indeks serwera dla tego klienta
   
   Http response(requestStr, serversCfg[serverIndex]); // użyj odpowiedniej konfiguracji serwera
      std::string responseStr = response.responseBuilder();
      int byteSend = send(pollFDs[el].fd, responseStr.c_str(), responseStr.size(), 0); // check if string functions are ok
      
      if (byteSend == -1)
      {
         std::cerr << "send() failed: " << strerror(errno) << std::endl;
         // Wyślij 500 Internal Server Error
         HttpError errorHandler;
         std::string errorResponse = errorHandler.generateErrorResponse(500);
         send(pollFDs[el].fd, errorResponse.c_str(), errorResponse.size(), 0);
         return;
      }
   std::string str = "Packet send sukcesfully!\n";
}

catch (const std::exception& e)
{
   // Jeśli cokolwiek pójdzie nie tak podczas parsowania lub generowania response
   std::cerr << "Exception in sendToClient: " << e.what() << std::endl;
   
   HttpError errorHandler;
   std::string errorResponse = errorHandler.generateErrorResponse(500,
   "An internal error occurred while processing your request.");
   send(pollFDs[el].fd, errorResponse.c_str(), errorResponse.size(), 0);
}
   pollFDs[el].events = POLLIN;
   // std::cout << "client: " << pollFDs[el].fd << " ready to send" << std::endl;
}