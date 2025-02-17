#include "../../include/Server.hpp"

void Server::topicCommand(int fd, std::string request)
{
    std::cout <<fd << request << std::endl;
}