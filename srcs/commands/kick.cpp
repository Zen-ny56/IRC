#include "../../include/Client.hpp"
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
        client->write(target->ClientNickname() + " is not in this channel.\r\n");
        return;
    }

    std::string message = ":" + client->ClientNickname() + " KICK " + channel->getName() + " " + target->ClientNickname() + " :" + reason + "\r\n";
    channel->broadcast(message);
    channel->removeClient(target);
}

std::vector<std::string> Server::split(const std::string& str, char delimiter)
{
    std::vector<std::string> tokens;
    std::stringstream ss(str);
    std::string token;

    while (std::getline(ss, token, delimiter))
    {
        tokens.push_back(token);
    }

    return tokens;
}

Channel* Server::getChannel(const std::string& channelName) {
    // Use explicit iterator type (std::map<std::string, Channel>::iterator)
    std::map<std::string, Channel>::iterator it = channels.find(channelName);
    
    // If the channel is found in the map, return a pointer to the Channel object
    if (it != channels.end()) {
        return &(it->second); // Return pointer to the Channel object
    }

    // Return NULL if the channel is not found
    return NULL;
}

Client* Server::getClient(int fd)
{
    // Iterate over the vector of clients
    for (std::vector<Client>::iterator it = clients.begin(); it != clients.end(); ++it)
    {
        // If the client fd matches, return the client object
        if (it->getFd() == fd)
            return &(*it);  // Return a pointer to the client object (dereferencing the iterator)
    }

    // Return nullptr if no client with the given fd is found
    return NULL;  // Return nullptr instead of creating a default Client object
}


void Server::kickCommand(int fd, const std::string& message) {
    Client* client = getClientByFd(fd); // Get the client by fd
    if (!client) {
        send(fd, "ERROR :You are not connected.\r\n", 30, 0);
        return;
    }

    std::vector<std::string> tokens = split(message, ' ');
    if (tokens.size() < 3) {
        std::string errormsg = "461 KICK :Not enough parameters\r\n";
        send(fd, errormsg.c_str(), errormsg.size(), 0);
        return;
    }

    std::string channelName = tokens[1];
    std::string targetNick = tokens[2];
    std::string reason = tokens.size() > 3 ? tokens[3] : "No reason given";

    // Use getNickname to find the target client by nickname (fd)
    Client* target = getClient(fd);
    if (!target) {
        std::string errormsg = "401 " + targetNick + " :No such nick/channel\r\n";
        send(fd, errormsg.c_str(), errormsg.size(), 0);
        return;
    }

    // Use getChannel to find the channel by its name
    Channel* channel = getChannel(channelName);
    if (!channel) {
        std::string errormsg = "403 " + channelName + " :No such channel\r\n";
        send(fd, errormsg.c_str(), errormsg.size(), 0);
        return;
    }

    // Call the kick function if the client is valid and the channel exists
    kick(client, target, reason);
}
