#include "../../include/Server.hpp"

void Server::sendCapabilities(int fd)
{
	std::string capMessage = "CAP * LS :multi-prefix sasl\r\n";
	send(fd, capMessage.c_str(), capMessage.size(), 0);
}

void Server::validatePassword(int fd, const std::string& message)
{
	if (message.rfind("PASS", 0) == 0)
	{ // Check if message starts with "PASS"
		std::vector<Client>::iterator it = getClient(fd);
		if (it == clients.end())
			throw std::runtime_error("No client was found\n");
		Client& client = (*this)[it];
		std::string receivedPassword = message.substr(5); // Extract password
		receivedPassword.erase(0, receivedPassword.find_first_not_of(" \t\r\n")); // Remove leading whitespace
		receivedPassword.erase(receivedPassword.find_last_not_of(" \t\r\n") + 1); // Remove trailing whitespace
		if (client.getPassAuthen() == true)
		{
			std::string errMsg = std::string(RED) + "462 PASS: You may not register\r\n" + std::string(WHI);
			send(fd, errMsg.c_str(), errMsg.size(), 0);
			return ;
		}
		if (receivedPassword.empty())
		{
			std::string errMsg = std::string(RED) + "461 PASS :Not enough parameters\r\n" + std::string(WHI);
			send(fd, errMsg.c_str(), errMsg.size(), 0); // ERR_NEEDMOREPARAMS
			return ;
		}
		if (receivedPassword.compare(this->password) == 0)
		{
			client.setPassAuthen();
			return ; // Authentication successful
		} 
		else
        {
			std::string errMsg = std::string(RED) + "464 :Password incorrect\r\n" + std::string(WHI);
			send(fd, errMsg.c_str(), errMsg.size(), 0); // ERR_PASSWDMISMATCH
			return ;
        }
	}
	return ; // Authentication failed
}

void Server::processNickUser(int fd, const std::string& message)
{
	// NICK command
	if (message.rfind("NICK ", 0) == 0)
	{
		std::vector<Client>::iterator it = getClient(fd);
		if (it == clients.end())
			throw  std::runtime_error("Client was not found\n");
		Client& client = (*this)[it];
		if (client.getPassAuthen() == false || client.getNickAuthen() == true)
			return;
		std::string nickname = message.substr(5); // Extract nickname
		nickname.erase(0, nickname.find_first_not_of(" \t\r\n"));
		nickname.erase(nickname.find_last_not_of(" \t\r\n") + 1);
		if (nickname.empty())
		{
			std::string errorMsg = std::string(RED) + "431 :No nickname given" + "\r\n" + std::string(WHI);
            send(fd, errorMsg.c_str(), errorMsg.size(), 0); // ERR_NONICKNAMEGIVEN
			return;
		}
		if (!isValidNickname(nickname))
		{
            std::string errorMsg = std::string(RED) + "432 " + nickname + " :Erroneous nickname" + "\r\n" + std::string(WHI); // ERR_ERRONEUSNICKNAME
			send(fd, errorMsg.c_str(), errorMsg.length(), 0);
			return;
		}
		if (nicknameMap.find(nickname) != nicknameMap.end())
		{
 			std::string errorMsg = std::string(RED) + "433 " + nickname + " :Nickname is already in use\r\n" + std::string(WHI); // ERR_NICKNAMEINUSE
			send(fd, errorMsg.c_str(), errorMsg.length(), 0);
			return;
		}
		// Update client's nickname
		// Client& client = getClient(fd);
		std::string oldNickname = client.getNickname();
		if (!oldNickname.empty())
			nicknameMap.erase(oldNickname); // Remove old nickname from the map
		client.setNickname(nickname);
		nicknameMap[nickname] = fd; // Add the new nickname to the map
		std::string response = std::string(GRE) + ":" + oldNickname + " NICK " + client.getNickname() +  "\r\n" + std::string(WHI); // Inform the client of the nickname change
		send(fd, response.c_str(), response.length(), 0);
		std::cout << "Client <" << fd << "> changed nickname to: " << nickname << std::endl;
	}
}

void Server::processUser(int fd, const std::string& message)
{
	// Split the message into parts
	std::vector<Client>::iterator it = getClient(fd);
	if (it == clients.end())
		throw std::runtime_error("Client was not found]\n");
	Client& client = (*this)[it];
	if (client.getNickAuthen() == false || client.getPassAuthen() == false)
	{
		std::cout << RED << "Entering Here" << WHI << std::endl;
		return ;
	}
	std::istringstream iss(message);
	std::vector<std::string> parts;
	std::string part;
	while (std::getline(iss, part, ' '))
        parts.push_back(part);
    // Check minimum parameter count
    if (parts.size() < 5 || parts[0] != "USER")
	{
		std::string errMsg = std::string(RED) +  "461 USER :Not enough parameters\r\n" + std::string(WHI);
		send(fd, errMsg.c_str(), errMsg.size(), 0); // ERR_NEEDMOREPARAMS
		return;
	}
	std::string username = parts[1];
	std::string unused1 = parts[2]; // This is usually "0"
	std::string unused2 = parts[3]; // This is usually "*"
	std::string realname = message.substr(message.find(':') + 1);

	// Check if the user is already registered
	if (client.getUserAuthen() == true)
	{
		std::string errMsg = std::string(RED) + "462 :You may not register\r\n" + std::string(WHI);
		send(fd, errMsg.c_str(), errMsg.size(), 0); // ERR_ALREADYREGISTERED
		return;
	}
	if (username.empty() || realname.empty())
	{
		std::string errMsg = std::string(RED) +  "461 USER :Not enough parameters\r\n" + std::string(WHI);
		send(fd, errMsg.c_str(), errMsg.size(), 0); // ERR_NEEDMOREPARAMS
		return;
	}
	// Register the user
	client.setUserName(username, realname);
	// Log successful processing
	this->sendWelcome(fd, client);
}

void Server::processCapReq(int fd, const std::string& message)
{
	if (message.find("CAP REQ") != std::string::npos){
		std::string capAck = "CAP * ACK :multi-prefix sasl\r\n";
		send(fd, capAck.c_str(), capAck.size(), 0);
	}
}

void Server::processQuit(int fd, const std::string& reason) 
{
    std::string nickname = clients[fd].getNickname();
	// Compose the QUIT message
	std::string quitMessage = ":" + nickname + " QUIT :Quit: " + (reason.empty() ? "" : reason);
	// Notify all clients sharing channels with the quitting client
	// broadcastToSharedChannels(fd, quitMessage); // Assume this function broadcasts to all relevant clients
    // std::string errorMessage = "ERROR :Closing link (" + nickname + ") [Quit: " + reason + "]";
	// send(fd, quitMessage.c_str(), quitMessage.size(), 0);
    // Remove the client from the server
    disconnectClient(fd); // Assume this function handles removing the client from all data structures and closing the connection
}

void Server::processPrivmsg(int fd, const std::string& message)
{
	std::vector<Client>::iterator bt = getClient(fd);
	if (bt == clients.end())
		throw std::runtime_error("Error finding client\n");
	Client& sender = (*this)[bt];
    size_t commandEnd = message.find(' ');
    if (commandEnd == std::string::npos || message.substr(0, commandEnd) != "PRIVMSG") {
        std::string error = std::string(RED) + "421: Unknown command\r\n" + std::string(WHI);
        send(fd, error.c_str(), error.size(), 0); // ERR_UNKNOWNCOMMAND
        return;
    }
    // Skip the "PRIVMSG" part
    size_t targetStart = commandEnd + 1; // Position after "PRIVMSG "
    size_t spacePos = message.find(' ', targetStart); // Find space after target
    if (spacePos == std::string::npos) {
        // If there's no space after the target, no message text is provided
        std::string error = std::string(RED) + "411: No recipient given (PRIVMSG)\r\n" + std::string(WHI);
        send(fd, error.c_str(), error.size(), 0); // ERR_NORECIPIENT
        return;
    }
    // Extract the target
    std::string target = message.substr(targetStart, spacePos - targetStart);
    // Skip any spaces after the target and check for message text
    size_t textStart = message.find_first_not_of(' ', spacePos + 1);
    if (textStart == std::string::npos) {
        // If no text is found after the target, return an error
        std::string error = std::string(RED) + "412: No text to send\r\n" + std::string(WHI);
        send(fd, error.c_str(), error.size(), 0); // ERR_NOTEXTTOSEND
        return;
    }
    // Extract the actual message text
    std::string text = message.substr(textStart);
	if (target[0] == '#')
	{
		std::map<std::string, Channel>::iterator it = channels.find(target);
		if (it == channels.end())
		{
			std::string error = std::string(RED) + "404 Cannot send to channel " + target + "\r\n" + std::string(WHI);
			send(fd, error.c_str(), error.size(), 0); // ERR_CANNOTSENDTOCHAN
			return;
		}
		// Check if the user is banned or not allowed in the channel
		Channel& channel = it->second;
		if (!channel.isInChannel(fd) || channel.isBanned(sender.getNickname()))
		{
			std::string error = std::string(RED) + "404 Cannot send to channel " + target + "\r\n" + std::string(WHI);
			send(fd, error.c_str(), error.size(), 0); // ERR_CANNOTSENDTOCHAN
			return;
		}
		// Send the message to the channel members
		channel.broadcastToChannel(text);
		}
		// std::map<std::string, Channel>::iterator it = channels.find(target);
	/*}*/ else
	{
		std::vector<Client>::iterator ct = getClientUsingNickname(target);
		if (ct == clients.end())
		{
			std::string error = std::string(RED) + "401: No such nickname " + target + "\r\n" + std::string(WHI);
            send(fd, error.c_str(), error.size(), 0); // ERR_NOSUCHNICK
			return ;
		}
		Client& recepient = (*this)[ct];
        // Send the private message to the user
        std::string response = ":" + sender.getNickname() + " PRIVMSG " + recepient.getNickname() + " :" + text + "\r\n";
        send(recepient.getFd(), response.c_str(), response.size(), 0);
    } 
}

void Server::processSasl(int fd, const std::string& message)
{
	if (message.find("AUTHENTICATE PLAIN") != std::string::npos)
	{
		std::string response = "AUTHENTICATE +\r\n";
		send(fd, response.c_str(), response.size(), 0);
    } else if (message.find("AUTHENTICATE ") == 0) {
		// Decode and validate credentials
		std::string credentials = message.substr(13); // Base64-encoded
		// Decode and verify (requires base64 decoding)
		// Example: Validate "username\0username\0password"
		send(fd, "900 :Authentication successful\r\n", 33, 0);
	}
}

void Server::capEnd(int fd)
{
	std::string capEnd = "CAP END\r\n";
	send(fd, capEnd.c_str(), capEnd.size(), 0);
}