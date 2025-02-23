#include "../../include/Server.hpp"

void Server::topicCommand(int fd, std::string request)
{
    Channel* channel = getClient(fd)->getChannel();
    channel->setTopic(request);
    std::cout << channel->getTopic() << request << std::endl;
}