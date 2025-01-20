#include "Channel.hpp"

Channel::Channel(const std::string& channelName): channelName(channelName)
{
}

void Channel::addClient(int fd)
{
    clientFds.push_back(fd);
}
