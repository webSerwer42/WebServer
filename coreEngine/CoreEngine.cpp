//Copyright [2025] <Piotr Ruszkiewicz> <pruszkie@student.42warsaw.pl>

#include "CoreEngine.hpp"
#include "../errors/error.hpp"

CoreEngine::CoreEngine(const std::vector<ServerConfig> &serversCfg) : serversCfg(serversCfg), serv(NULL), 
    lSockNum(0), poolTimeout(1000), pollFDsNum(0), backlogNum(128)
{
   memset(&hints, 0, sizeof(sockaddr_in));
   hints.ai_family = AF_UNSPEC;     // for both IP4 & IP6
   hints.ai_socktype = SOCK_STREAM; // TCP
   hints.ai_flags = AI_PASSIVE;     // for server means NULL will accept connection from any adress
}

void CoreEngine::setSocket(size_t i)
{
   // this is creation of listening sockets, pushing to vector
   // socketFD.push_back(socket(serv->ai_family, SOCK_STREAM, 0)); // error check missing
   // release soket from kernel hold, allow to multile use after close
   int FD = socket(serv->ai_family, SOCK_STREAM, 0);
   if(FD == -1)
   {
      std::cerr << "socket() failed: " << strerror(errno) << std::endl;
      return;
   }
   socketFD.push_back(FD);
   int option = 1;
   if (setsockopt(socketFD[i], SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option)) == -1)
   {
      std::cerr << "setsockopt() failed: " << strerror(errno) << std::endl;
      return;
   }
   // setting AF_INET6 wasnt specified in subect but i did it as a real world case scenario
   if (serv->ai_family == AF_INET6)
   {
      int yes = 1;
      // Make this IPv6 socket handle only IPv6 addresses, preventing conflicts with IPv4 sockets
      if (setsockopt(socketFD[i], IPPROTO_IPV6, IPV6_V6ONLY, &yes, sizeof(yes)) == -1)
      {
         std::cerr << "setsockopt() failed: " << strerror(errno) << std::endl;
         close(socketFD[i]);
         return;
      }
   }
   // this function set sockets to be non-blocking, connection will not hold program from further execution
   fcntl(socketFD[i], F_SETFL, O_NONBLOCK);
   /*binding socket FD to IP+port stored in erv->ai_addr, similar FD <-> pipe end;
   casting to defoult struct sockaddr that can work with any kind of those structs*/
   std::cout << "socketFD: " << socketFD[i] << std::endl;
   if (bind(socketFD[i], serv->ai_addr, serv->ai_addrlen) == -1)
   {
      std::cerr << "bind() failed: " << strerror(errno) << std::endl;
      close(socketFD[i]);
      return;
   }
   // last param for listen in number of connection allowed on the incoming queue
   if (listen(socketFD[i], backlogNum) == -1)
   {
      std::cerr << "listen() failed: " << strerror(errno) << std::endl;
      close(socketFD[i]);
      return;
   }
}

void CoreEngine::coreEngine()
{
   // this function will be adjusted to data provided after parsing config file
   size_t j = 0;
   // initializig listeling sockets by every config 
   for(size_t i = 0; i < serversCfg.size(); i++)
   {
      int status = getaddrinfo(serversCfg[i].host.c_str(), serversCfg[i].listen_port.c_str(), &hints, &serv);
      if (status != 0)
      {
         std::cerr << "getaddrinfo() failed for server " << i << ": " << gai_strerror(status) << std::endl;
         if (serv != NULL) {
            freeaddrinfo(serv);
            serv = NULL;
         }
         continue; // skip this server and try next one
      }
      std::cout << "server nr: " << i << std::endl;
      addrinfo *res = serv;
      while (true)
      {
         setSocket(j);
         // creating a pararel vector to pollFD just to distiquish listening from client socket
         isClientFD.push_back(false);
         // serverFDtoIndex[socketFD[j]] = i; // mapowanie server FD -> config index
         lSockNum++;
         // realloc is nessesary due to dynamics changes to pollfd array, it holds all sockets events data 
         // pollFDs = (pollfd *)realloc(pollFDs, (lSockNum + 1) * sizeof(pollfd));
         pollfd pfd;
         pfd.fd = socketFD[j];
         pfd.events = POLLIN;
         pollFDs.push_back(pfd);
         j++;
         if (serv->ai_next == NULL)
            break;
         serv = serv->ai_next;
      }
      freeaddrinfo(res);
   }

   std::cout << "number of listening sockets: " << lSockNum << std::endl;

   pollFDsNum = lSockNum;
   while (pollFDsNum > 0)
   {
      int pollstatus = poll(&(pollFDs[0]), pollFDsNum, poolTimeout);
      if (pollstatus > 0)
      {
         for (size_t i = 0; i < pollFDsNum; i++)
         {
            // std::cout << "Socket: " << pollFDs[i].fd << std::endl;
            // only on listening sockets, after first request
            if ((pollFDs[i].revents & POLLIN) && (i < lSockNum) && !(isClientFD[i]))
               setConnection(i);
            else if (pollFDs[i].revents & POLLIN)
               recivNClose(i);
            // std::cout << "socket: " << pollFDs[i].fd << " revent: " << pollFDs[i].revents << std::endl;
            if (pollFDs[i].revents & POLLOUT)
               sendToClient(i);
            if (pollFDs[i].revents & POLLHUP) // consider it coz might be unnesesary.
            {
               std::cout << "client on FD: " << pollFDs[i].fd << " has disconnected" << std::endl;
               closeCLient(i);
               continue;
            }
            if (pollFDs[i].revents & POLLERR)
            {
               // means connection is broken, no point to send error to client
               std::cout << "poll() failed: revent return POLLERR on FD: " << pollFDs[i].fd << std::endl;
               closeCLient(i);
               continue;
            }
         }
      }
      // usleep(100000);
   }
}