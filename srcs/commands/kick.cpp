#include "../../include/Server.hpp"
#include "../../include/Channel.hpp"

void Server::kick(Client* client, Client* target, const std::string& reason)
{
    Channel* channel = client->getChannel();
    if (!channel)
    {
        client->write("You are not in a channel.\r\n");
        return;
    }

    if (channel->getAdmin() != client)
    {
        client->write("You are not the admin of this channel.\r\n");
        return;
    }

    if (!channel->isInChannel(target->getFd()))
    {
        client->write(target->getNickname() + " is not in this channel.\r\n");
        return;
    }

    std::string message = ":" + client->getNickname() + " KICK " + channel->getName() + " " + target->getNickname() + " :" + reason + "\r\n";
    channel->broadcast(message);
    channel->removeClient(target);
}

void Server::kickCommand(int fd, const std::string& message)
{
    Client* client = getClientByFd(fd);
    if (!client)
    {
        send(fd, "ERROR :You are not connected.\r\n", 30, 0);
        return;
    }

    std::vector<std::string> tokens = split(message, ' ');
    if (tokens.size() < 3)
    {
        std::string errormsg = "461 KICK :Not enough parameters\r\n";
        send(fd, errormsg.c_str(), errormsg.size(), 0);
        return;
    }

    std::string channelName = tokens[1];
    std::string targetNick = tokens[2];
    std::string reason = tokens.size() > 3 ? tokens[3] : "No reason given";

    Client* target = getClientByNick(targetNick);
    if (!target)
    {
        std::string errormsg = "401 " + targetNick + " :No such nick/channel\r\n";
        send(fd, errormsg.c_str(), errormsg.size(), 0);
        return;
    }

    Channel* channel = getChannelByName(channelName);
    if (!channel)
    {
        std::string errormsg = "403 " + channelName + " :No such channel\r\n";
        send(fd, errormsg.c_str(), errormsg.size(), 0);
        return;
    }

    kick(client, target, reason);
}