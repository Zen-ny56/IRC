#include "../../include/Server.hpp"

void Server::sendUserList(int fd, Client& client, const std::string& channelName, Channel& channel)
{
    // Prepare the list of clients in the channel
    std::vector<int> clientList = channel.listUsers(); // List of FD (or IDs) of users in the channel
    
    // Prepare the initial part of the message with the channel name
    std::string userListMessage = std::string(YEL) + "353 " + client.ClientNickname() + " = " + channelName + " :";
    
    // Iterate through the client list and append each client's nickname
    bool firstClient = true; // To manage the format of the list (comma-separated)
    for (std::vector<int>::iterator it = clientList.begin(); it != clientList.end(); ++it)
    {
        std::vector<Client>::iterator clientIter = relayClient(*it); // Get the client iterator using their FD
        
        if (clientIter != clients.end()) // Ensure the client is valid
        {
            Client& user = (*this)[clientIter];
            
            // Avoid comma before the first user in the list
            if (!firstClient)
                userListMessage += " "; // Add a space between nicknames
            else
                firstClient = false; // After the first nickname, start adding spaces between nicknames
                
            userListMessage += user.ClientNickname(); // Append the nickname of the client
        }
    }

    // Send the list of users to the client
    send(fd, userListMessage.c_str(), userListMessage.size(), 0);
    
    // Send the end-of-names message
    std::string endOfListMessage = std::string(YEL) + "366 " + client.ClientNickname() + " " + channelName + " :End of /Names list\r\n" + std::string(WHI);
    send(fd, endOfListMessage.c_str(), endOfListMessage.size(), 0);
}

void Server::sendChannelTopic(int fd, Client& client, const std::string& channelName, Channel& channel)
{
    // Check if the channel has a topic set
    if (!channel.getTopic().empty())
    {
        // Send the topic to the client
        std::string topicMessage = std::string(YEL) + "332 " + client.ClientNickname() + " " + channelName + " :" + channel.getTopic() + "\n" + std::string(WHI);
        send(fd, topicMessage.c_str(), topicMessage.size(), 0);
    }
    else
    {
        // If no topic is set, send a message indicating that
        std::string noTopicMessage = std::string(YEL) + "331 " + client.ClientNickname() + " " + channelName + " :No topic is set\r\n" + std::string(WHI);
        send(fd, noTopicMessage.c_str(), noTopicMessage.size(), 0);
    }
}


void Server::joinChannel(int fd, const std::string& channelName, const std::string& key)
{
    std::vector<Client>::iterator iter = relayClient(fd);
    if (iter == clients.end())
        throw std::runtime_error("Error finding client\n");

    Client& client = (*this)[iter];

    // If client is not authenticated, do not allow joining
    if (client.getUserAuthen() == false)
        return;

    // Find the channel
    std::map<std::string, Channel>::iterator it = channels.find(channelName);
    if (it == channels.end())
    {
        // Channel doesn't exist, create it
        channels[channelName] = Channel(channelName, key);
        it = channels.find(channelName);  // Re-get the iterator after creation
    }

    Channel& channel = it->second;

    // Check if the client is already in the channel
    if (channel.isInChannel(fd))
    {
        // Send message that the user is already in the channel
        std::string msg = std::string(RED) + client.ClientNickname() + " " + channelName + " :You are already in this channel\n" + std::string(WHI);
		std::cout << msg << std::endl;
        send(fd, msg.c_str(), msg.size(), 0);
        return; // Exit, no need to add client again
    }

    // Add client to the channel
    channel.addClient(&client);

    // Send the success message
    std::cout << GRE << std::setw(15) << "|-- Successfully joined Channel: " << channelName << EN << std::endl;

    // Broadcast JOIN message to all clients in the channel
    std::string joinMessage = ":" + client.ClientNickname() + " JOIN :" + channelName + "\r\n" + std::string(WHI);
    channel.broadcastToChannel(joinMessage);

    // Send the channel topic or indicate no topic
    sendChannelTopic(fd, client, channelName, channel);

    // Send the list of users in the channel
    sendUserList(fd, client, channelName, channel);
}

void Server::handleChannel(int fd, const std::string& message)
{
    // Extract parameters after JOIN, client is going to send JOIN #channel1,#channel2 key1,key2 or JOIN #channel1
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

    // Iterate through each channel and call joinChannel
    for (size_t i = 0; i < channels.size(); ++i)
    {
        const std::string& channelName = channels[i];
        const std::string& key = (i < keys.size()) ? keys[i] : ""; // Match keys to channels if possible

        if (!isValidChannelName(channelName))
        {
            std::string errormsg = std::string(RED) + "476 " + channelName + " :Invalid channel name" + std::string(EN) + "\r\n";
            send(fd, errormsg.c_str(), errormsg.size(), 0); // ERR_BADCHANMASK
            continue;
        }
        else
        {
            // Attempt to join the channel
            joinChannel(fd, channelName, key);
        }
    }
}
