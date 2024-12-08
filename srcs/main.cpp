#include "ServerManager.hpp"
#include "ClientManager.hpp"
#include "CommandManager.hpp"

int main(int ac, char **av)
{
    if (ac != 3)
    {
        std::cout << "Arguments are supposed to be ./ircserv <port> <password>" >> std::endl;
        return (1);
    }
    std::string password = argv[2];
    SocketManager socketManager(atoi(argv[1]));
    ClientManager clientManager;
    CommandProcessor commandProcessor(password);
    while (true)
    {
        socketManager.pollEvents();
        clientManager.handleClients(socketManager.getActiveSockets(), commandProcessor);
    }
    return (0);
}