#include "../../include/Server.hpp"
#include "../../include/Channel.hpp"

void Server::kickCommand(int fd, const std::string& message)
{
    // Extract parameters after KICK command
    size_t spacePos = message.find(' ');
    if (spacePos == std::string::npos || spacePos + 1 >= message.length())
    {
        std::string errormsg = std::string(RED) + "461 KICK :Not enough parameters\r\n";
        send(fd, errormsg.c_str(), errormsg.size(), 0); // ERR_NEEDMOREPARAMS
        return;
    }

    std::string params = message.substr(spacePos + 1);

    // Split channels and keys
    size_t nextSpace = params.find(' ');
    std::string channelsPart = params.substr(0, nextSpace);
    std::string keysPart = (nextSpace != std::string::npos) ? params.substr(nextSpace + 1) : "";

    std::vector<std::string> channels = splitByDelimiter(channelsPart, ',');
    std::vector<std::string> keys = splitByDelimiter(keysPart, ',');

    std::vector<std::string> validChannels;

    // Iterate through each channel and validate
    for (size_t i = 0; i < channels.size(); ++i)
    {
        std::string& input = channels[i];
        std::string client, target;
        std::string parsedMessage;
        const std::string& key = (i < keys.size()) ? keys[i] : ""; // Match keys to channels

        if (!checkParsing(input, client, target, parsedMessage))
        {
            std::string errormsg = std::string(RED) + "476 " + input + " :Bad channel mask\r\n";
            send(fd, errormsg.c_str(), errormsg.size(), 0);
            continue;
        }

        validChannels.push_back(input);
    }

}

bool Server::checkParsing(std::string& input, std::string client, std::string target, std::string message)
{

    if (input.size() != 4)
    {
        client = input.substr(1);
        target = input.substr(2);
        if (!input[3])
            message = "No reason given";
        else
            message = input.substr(3);
        return true;  
    }
    return false;
}