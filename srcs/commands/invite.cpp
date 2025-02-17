#include "../../include/Server.hpp"

void Server::inviteCommand(int fd, std::string command)
{
    std::cout << fd << command << "Inviting from here" << std::endl;
}