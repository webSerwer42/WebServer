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
   // recived date is send to buffer, for now its strign
   client &client = this->getClientByFD(pollFDs[el].fd);
   HttpError errorHandler;
   client.byteRecived = recv(pollFDs[el].fd, client.inputBuffer, 1024, 0);
   if (client.byteRecived >= 0)
      client.inputBuffer[client.byteRecived] = '\0';
   if (client.byteRecived < 0)
   {
      std::cerr << "recv() failed: " << strerror(errno) << std::endl;
      std::string errorResponse = errorHandler.generateErrorResponse(500);
      client.hasError = true;
      client.sendBuffer = errorResponse;
      prepareResponse(client, el);
      return;
   }
   // this is closing socket logic, when send EOF by client EOF
   else if (client.byteRecived == 0)
      closeCLient(el);
   else
   {
      // czy request jest pusty (400)
      std::string requestStr(client.inputBuffer);
      if (requestStr.empty() || requestStr.find("HTTP") == std::string::npos)
      {
         std::cout << "Empty or invalid HTTP request detected!" << std::endl;
         std::string errorResponse = errorHandler.generateErrorResponse(400,
            "The request is empty or does not contain valid HTTP headers.");
         client.hasError = true;
         client.sendBuffer = errorResponse;
         prepareResponse(client, el);
         return;
      }
      // czy buffer jest za mały (413)
      if (client.byteRecived == (int)(sizeof(client.inputBuffer) - 1))
      {
         std::cout << "Request too large! Buffer full." << std::endl;
         std::string errorResponse = errorHandler.generateErrorResponse(413,
            "The request payload exceeds the maximum buffer size of 1024 bytes.");
         client.hasError = true;
         client.sendBuffer = errorResponse;
         prepareResponse(client, el);
         return;
      }
      // collecting chunks of recived data
      std::string temp(client.inputBuffer);
      client.requestBuffer += temp; 
      size_t index = client.requestBuffer.find("\r\n\r\n");
      if(index != std::string::npos)
         prepareResponse(client, el);
   }
}

void CoreEngine::sendToClient(size_t el)
{
   client &client = this->getClientByFD(pollFDs[el].fd);
   std::cout << "--->this is request buffer: " << client.requestBuffer << std::endl;
   std::cout << "actual client: " << client.FD << std::endl;
   try
   {
      //Http response(client.sendBuffer);
      // Obsluga erroruw w Http class
      // std::string responseStr to object.responseBuilder();
      // std::cout << "---> What is response: " << object.getResponse() << std::endl;
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
      if((0 < byteSend) && (byteSend < (int)client.sendBuffer.size()))
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
   client.requestBuffer.clear();
   client.sendBuffer.clear();
   client.sendOffset = 0;
   pollFDs[el].events = POLLIN;
   // std::cout << "client: " << pollFDs[el].fd << " ready to send" << std::endl;
}

/*
-keep full requestBuffer 
-kepp leftover for next 
-flag POLLOUT for sending 
-send first request -clean requestBuffer 
-flag to POLLIN 
-move leftover to requestBuffer 
-continue with reciving
*/