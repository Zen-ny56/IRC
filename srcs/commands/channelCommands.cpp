#include "../../include/Server.hpp"

void Server::handleChannel(int fd, const std::string& message)
{
	//Extract parameters after JOIN , client is going to send JOIN #channel1,#channel2 key1,key2 or JOIN #channel1
	size_t paramsStart = message.find(' ') + 1;
    if (paramsStart == std::string::npos || paramsStart >= message.length())
	{
		std::string errormsg = std::string(RED) + "461 JOIN :Not enough parameters\r\n";
		send(fd, errormsg.c_str(), errormsg.size(), 0); // ERR_NEEDMOREPARAMS
		return;
	}
	std::string params = message.substr(paramsStart);
	// Split channels and keys
	size_t spacePos = params.find(' ');
	std::string channelsPart = params.substr(0, spacePos); // Comma-separated channel names
	std::string keysPart = (spacePos != std::string::npos) ? params.substr(spacePos + 1) : ""; // Comma-separated keys
	// Parse channels and keys
	std::vector<std::string> channels = splitByDelimiter(channelsPart, ',');
	std::vector<std::string> keys = splitByDelimiter(keysPart, ',');

	//Iterate through each channel and call joinChannel
	for (size_t i = 0; i < channels.size(); ++i)
	{
		const std::string& channelName = channels[i];
		const std::string& key = (i < keys.size()) ? keys[i] : ""; // Match keys to channels if possible
        if (!isValidChannelName(channelName))
		{
			std::string errormsg = std::string(RED) + "476 " + channelName + " :Invalid channel name\r\n";
			send(fd, errormsg.c_str(), errormsg.size(), 0); // ERR_BADCHANMASK
			continue;
		}
		joinChannel(fd, channelName, key);
    }
}

void Server::joinChannel(int fd, const std::string& channelName, const std::string& key)
{
	std::vector<Client>::iterator iter = getClient(fd);
	if (iter == clients.end())
		throw std::runtime_error("Error finding client\n");
	Client& client = (*this)[iter];
	if (client.getUserAuthen() == false)
		return ;
	std::map<std::string, Channel>::iterator it = channels.find(channelName);
	if (it == channels.end())
	{
		// Channel doesn't exist, so create it
		channels[channelName] = Channel(channelName, key);
		it = channels.find(channelName); // Re-get the iterator after creation
	}
	Channel& channel = it->second;
	if (channel.isInChannel(fd))
		return ;
    // 3. Validate conditions for joining the channel
	if (channel.isInviteOnly() && !channel.isInvited(fd))
	{
		std::string errorMsg = std::string(RED) + "473 " + client.getNickname() + " " + channelName + " :Invite-only channel\r\n" + std::string(WHI);
		send(fd, errorMsg.c_str(), errorMsg.size(), 0);
		return;
    }
    if (channel.isFull())
	{
		std::string errorMsg = std::string(RED) + client.getNickname() + " " + channelName + " :Channel is full\r\n" + std::string(WHI);
		send(fd, errorMsg.c_str(), errorMsg.size(), 0);
		return;
	}
	if (!channel.getKey().empty() && channel.getKey() != key)
	{
		std::string errorMsg = std::string(RED) + "475" + client.getNickname() + " :Incorrrect channel key\r\n" + std::string(WHI);
		send(fd, errorMsg.c_str(), errorMsg.size(), 0);
		return;
    }
	if (channel.isBanned(client.getNickname()))
	{
		std::string errorMsg = std::string(RED) + "474" + client.getNickname() + " :You are banned from this channel\r\n" + std::string(WHI);
        send(fd, errorMsg.c_str(), errorMsg.size(), 0);
        return;
    }
	
	// 4. Add the client to the channel
	channel.addClient(fd);
	
	// 5. Broadcast JOIN message to all clients in the channel
	std::string joinMessage = ":" + client.getNickname() + " JOIN :" + channelName + "\r\n" + std::string(WHI);
	channel.broadcastToChannel(joinMessage);

    // 6. Send the channel topic (or indicate no topic set)
    if (!channel.getTopic().empty())
	{
		std::string info = std::string(YEL) + "332 " + client.getNickname() + " " + channelName + " :" + channel.getTopic() + "\r\n" + std::string(WHI);
		send(fd, info.c_str() ,info.size(), 0);
	} else {
		std::string info = std::string(YEL) + "331 " + client.getNickname() + " " + channelName + " :No topic is set\r\n" + std::string(WHI);
		send(fd, info.c_str(), info.size(), 0);
    }

    // 7. Send the list of users in the channel
	std::vector<int> clientList = channel.listUsers();
	for (std::vector<int>::iterator it = clientList.begin(); it != clientList.end(); ++it)
	{
		std::vector<Client>::iterator bt = getClient(*it);
		if (bt == clients.end())
			throw std::runtime_error("Error finding clients\n");
		Client& user = (*this)[bt];
		std::string msg = std::string(YEL) + "353 " + client.getNickname() + " = " + channelName + " :" +  user.getNickname() + "\r\n" + std::string(WHI);
		send(fd, msg.c_str(), msg.size(), 0); 
	}
	std::string msg = std::string(YEL) + "366 " + client.getNickname() + " " + channelName + " :End of /Names list\r\n" + std::string(WHI);
    send(fd, msg.c_str(), msg.size(), 0);
}
