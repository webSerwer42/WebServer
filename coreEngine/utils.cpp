#include "CoreEngine.hpp"

CoreEngine::client& CoreEngine::getClientByFD(int socketFD)
{
    for(size_t i = 0; i < this->clientVec.size(); i++)
        if(clientVec[i].FD == socketFD)
            return clientVec[i];
    std::stringstream ss;
    ss << "Client not found for socket FD: " << socketFD;
    throw std::runtime_error(ss.str());
}

void CoreEngine::closeCLient(int el)
{
    close(pollFDs[el].fd);
      for (int i = el; i < (int)pollFDsNum; i++)
      {
         if (el == ((int)pollFDsNum - 1))
            break;
         pollFDs[i] = pollFDs[i + 1];
      }
      // shirinking pollfd
      pollFDs = (pollfd *)realloc(pollFDs, (pollFDsNum - 1) * sizeof(pollfd));
      pollFDsNum--;
}