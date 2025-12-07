#include "CoreEngine.hpp"

CoreEngine::client &CoreEngine::getClientByFD(int socketFD)
{
    for (size_t i = 0; i < this->clientVec.size(); i++)
        if (clientVec[i].FD == socketFD)
            return clientVec[i];
    std::stringstream ss;
    ss << "Client not found for socket FD: " << socketFD;
    throw std::runtime_error(ss.str());
}

void CoreEngine::closeCLient(int el)
{
    for (size_t i = 0; i < clientVec.size(); i++)
        if (clientVec[i].FD == pollFDs[el].fd)
        {
            clientVec.erase(clientVec.begin() + i);
            break;
        }
    close(pollFDs[el].fd);
    pollFDs.erase(pollFDs.begin() + el);
    pollFDsNum--;
}

std::string CoreEngine::prepareResponse(client &client, size_t el, size_t res)
{
    std::cout << "-->This is request: " << client.requestBufferVec[res] << std::endl;
    Http object(client.requestBufferVec[res], client.serverCfg);
    if (object.getIsError())
        client.hasError = true;
    pollFDs[el].events = POLLOUT;
    std::cout << "--> This is Response: " << object.getResponse().c_str() << std::endl;
    return object.getResponse();
}