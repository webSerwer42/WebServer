// Copyright [2025] <Piotr Ruszkiewicz> <pruszkie@student.42warsaw.pl>

#include "CoreEngine.hpp"

void CoreEngine::setConnection(size_t i)
{
   socklen_t addrLen = sizeof(sockaddr_storage);
   client client;
   memset(&client.clientSockaddr, 0, sizeof(sockaddr_in));
   client.serverCfg = serversCfg[i];
   client.hasError = false;
   client.sendOffset = 0;

   client.FD = accept(pollFDs[i].fd, (struct sockaddr *)&client.clientSockaddr, &addrLen);
   if (client.FD == -1)
   {
      std::cerr << "accept() failed: " << strerror(errno) << std::endl;
      exit(1);
   }
   pollfd pfd;

   // pollFDs = (pollfd *)realloc(pollFDs, (pollFDsNum + 1) * sizeof(pollfd));
   pfd.fd = client.FD;
   pfd.revents = 0;
   pollFDs.push_back(pfd);

   isClientFD[pollFDsNum] = true;
   // clientToServer[pollFDsNum] = serverFDtoIndex[pollFDs[i].fd]; // zapisz który serwer przyjął połączenie
   pollFDs[pollFDsNum].events = POLLIN;
   pollFDsNum++;
   this->clientVec.push_back(client);
   std::cout << "socket: " << pollFDs[i].fd << " ready to connect" << std::endl;
}

void CoreEngine::recivNClose(size_t el)
{
   client &client = this->getClientByFD(pollFDs[el].fd);
   HttpError errorHandler;

   client.byteRecived = recv(pollFDs[el].fd, client.inputBuffer, sizeof(client.inputBuffer), 0);

   if (client.byteRecived < 0)
   {
      std::cerr << "recv() failed: " << strerror(errno) << std::endl;
      client.hasError = true;
      client.sendOffset = 0;
      client.sendBuffer = errorHandler.generateErrorResponse(500);
      pollFDs[el].events = POLLOUT;
      return;
   }
   else if (client.byteRecived == 0)
   {
      closeCLient(el);
      return;
   }

   // WAŻNE: traktuj dane jako bajty (upload może zawierać '\0')
   client.leftooverBuffer.append(client.inputBuffer, client.byteRecived);

   // Opcjonalnie: limit na rozmiar bufora (żeby nie rosło w nieskończoność)
   // if (client.leftooverBuffer.size() > 10 * 1024 * 1024) { ... 413 ... }

   std::string &temp = client.leftooverBuffer;
   size_t begin = 0;

   while (true)
   {
      size_t end = temp.find("\r\n\r\n", begin);
      if (end == std::string::npos)
      {
         // czekamy na resztę nagłówków
         temp.erase(0, begin);
         break;
      }

      std::string headers = temp.substr(begin, end - begin + 4);

      // Minimalna walidacja dopiero, gdy mamy całe nagłówki
      if (headers.find("HTTP/") == std::string::npos)
      {
         std::cout << "Invalid HTTP headers detected!" << std::endl;
         client.hasError = true;
         client.sendOffset = 0;
         client.sendBuffer = errorHandler.generateErrorResponse(
            400, "The request does not contain valid HTTP headers."
         );
         pollFDs[el].events = POLLOUT;
         return;
      }

      size_t contentLength = 0;
      size_t clPos = headers.find("Content-Length:");
      if (clPos != std::string::npos)
      {
         clPos += 15;
         while (clPos < headers.size() && (headers[clPos] == ' ' || headers[clPos] == '\t')) clPos++;
         size_t clEnd = headers.find("\r\n", clPos);
         std::string clStr = headers.substr(clPos, clEnd - clPos);
         contentLength = static_cast<size_t>(std::strtoul(clStr.c_str(), NULL, 10));
      }

      size_t fullRequestLen = (end - begin + 4) + contentLength;

      if (temp.size() - begin < fullRequestLen)
      {
         // brakuje body
         temp.erase(0, begin);
         break;
      }

      std::string request = temp.substr(begin, fullRequestLen);
      client.requestBufferVec.push_back(request);
      begin += fullRequestLen;
   }

   if (!client.requestBufferVec.empty())
   {
      client.sendBuffer.clear();
      client.sendOffset = 0;

      for (size_t i = 0; i < client.requestBufferVec.size(); i++)
         client.sendBuffer += prepareResponse(client, el, i);

      client.requestBufferVec.clear();
      pollFDs[el].events = POLLOUT;
   }
}

void CoreEngine::sendToClient(size_t el)
{
   client &client = this->getClientByFD(pollFDs[el].fd);
   std::cout << "actual client: " << client.FD << std::endl;
   std::cout << "send buffer: " << client.sendBuffer << std::endl;
   try
   {
      // Obsluga erroruw w Http class
      int byteSend = send(pollFDs[el].fd, client.sendBuffer.c_str() + client.sendOffset,
                          client.sendBuffer.size() - client.sendOffset, 0); // check if string functions are ok
      if (byteSend <= 0)
      {
         std::cerr << "send() failed: " << strerror(errno) << std::endl;
         // Wyślij 500 Internal Server Error
         HttpError errorHandler;
         std::string errorResponse = errorHandler.generateErrorResponse(500);
         closeCLient(el);
         return;
      }
      if ((0 < byteSend) && (byteSend < (int)client.sendBuffer.size()))
      {
         client.sendOffset += byteSend;
         return;
      }
      if (client.hasError == true)
      {
         closeCLient(el);
         return;
      }
      client.sendOffset = 0;
   }
   catch (const std::exception &e)
   {
      // Jeśli cokolwiek pójdzie nie tak podczas parsowania lub generowania response
      std::cerr << "Exception in sendToClient: " << e.what() << std::endl;
      HttpError errorHandler;
      std::string errorResponse = errorHandler.generateErrorResponse(500,
                                                                     "An internal error occurred while processing your request.");
      client.hasError = true;
      client.sendBuffer = errorResponse;
      return;
   }
   client.sendBuffer.clear();
   client.sendOffset = 0;
   pollFDs[el].events = POLLIN;
   // std::cout << "client: " << pollFDs[el].fd << " ready to send" << std::endl;
}
