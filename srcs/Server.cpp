#include "Server.hpp"

Server::Server(){serSocketFd = -1;}

void Server::clearClients(int fd){ //-> clear the clients
	for(size_t i = 0; i < fds.size(); i++){ //-> remove the client from the pollfd
		if (fds[i].fd == fd)
			{fds.erase(fds.begin() + i); break;}
	}
	for(size_t i = 0; i < clients.size(); i++){ //-> remove the client from the vector of clients
		if (clients[i].getFd() == fd)
			{clients.erase(clients.begin() + i); break;}
	}

}

bool Server::signal = false; //-> initialize the static boolean

void Server::signalHandler(int signum)
{
	(void)signum;
	std::cout << std::endl << "Signal Received!" << std::endl;
	Server::signal = true; //-> set the static boolean to true to stop the server
}

void	Server::closeFds(){
	for(size_t i = 0; i < clients.size(); i++){ //-> close all the clients
		std::cout << RED << "Client <" << clients[i].getFd() << "> Disconnected" << WHI << std::endl;
		close(clients[i].getFd());
	}
	if (serSocketFd != -1){ //-> close the server socket
		std::cout << RED << "Server <" << serSocketFd << "> Disconnected" << WHI << std::endl;
		close(serSocketFd);
	}
}

void Server::receiveNewData(int fd)
{
	char buff[1024]; //-> buffer for the received data
	memset(buff, 0, sizeof(buff)); //-> clear the buffer

	ssize_t bytes = recv(fd, buff, sizeof(buff) - 1 , 0); //-> receive the data
	if(bytes <= 0)
	{ //-> check if the client disconnected
		std::cout << RED << "Client <" << fd << "> Disconnected" << WHI << std::endl;
		clearClients(fd); //-> clear the client
		close(fd); //-> close the client socket
	} else 
	{ //-> print the received data
		buff[bytes] = '\0';
		std::string message(buff);
		if (message.find("CAP LS") != std::string::npos)
			sendCapabilities(fd);
		else if (message.rfind("PASS ", 0) == 0)
			validatePassword(fd, message);
		else if ((message.rfind("NICK ", 0) == 0) || (message.rfind("USER ", 0) == 0))
			processNickUser(fd, message);
		else if (message.find("CAP REQ") != std::string::npos)
			processCapReq(fd, message);
		else if (message.find("AUTHENTICATE") != std::string::npos)
			processSasl(fd, message);
		else if (message.find("CAP END") != std::string::npos)
			capEnd(fd);
		else
			std::cout << YEL << "Client <" << fd << "> Data: " << WHI << buff;
	}
}

void Server::acceptNewClient()
{
	Client cli; //-> create a new client
	struct sockaddr_in cliadd;
	struct pollfd newPoll;
	socklen_t len = sizeof(cliadd);

	int incofd = accept(serSocketFd, (sockaddr *)&(cliadd), &len); //-> accept the new client
	if (incofd == -1)
		{std::cout << "accept() failed" << std::endl; return;}

	if (fcntl(incofd, F_SETFL, O_NONBLOCK) == -1) //-> set the socket option (O_NONBLOCK) for non-blocking socket
		{std::cout << "fcntl() failed" << std::endl; return;}

	newPoll.fd = incofd; //-> add the client socket to the pollfd
	newPoll.events = POLLIN; //-> set the event to POLLIN for reading data
	newPoll.revents = 0; //-> set the revents to 0

	cli.setFd(incofd); //-> set the client file descriptor
	cli.setIpAdd(inet_ntoa((cliadd.sin_addr))); //-> convert the ip address to string and set it
	clients.push_back(cli); //-> add the client to the vector of clients
	fds.push_back(newPoll); //-> add the client socket to the pollfd

	authenticatedClients[incofd] = false;
	std::cout << GRE << "Client <" << incofd << "> Connected" << WHI << std::endl;
}

void Server::serSocket()
{
	int en = 1;
	struct sockaddr_in add;
	struct pollfd newPoll;
	add.sin_family = AF_INET; //-> set the address family to ipv4
	add.sin_addr.s_addr = INADDR_ANY; //-> set the address to any local machine address
	add.sin_port = htons(this->port); //-> convert the port to network byte order (big endian)

	serSocketFd = socket(AF_INET, SOCK_STREAM, 0); //-> create the server socket
	if(serSocketFd == -1) //-> check if the socket is created
		throw(std::runtime_error("failed to create socket"));
	if(setsockopt(serSocketFd, SOL_SOCKET, SO_REUSEADDR, &en, sizeof(en)) == -1) //-> set the socket option (SO_REUSEADDR) to reuse the address
		throw(std::runtime_error("failed to set option (SO_REUSEADDR) on socket"));
	 if (fcntl(serSocketFd, F_SETFL, O_NONBLOCK) == -1) //-> set the socket option (O_NONBLOCK) for non-blocking socket
		throw(std::runtime_error("failed to set option (O_NONBLOCK) on socket"));
	if (bind(serSocketFd, (struct sockaddr *)&add, sizeof(add)) == -1) //-> bind the socket to the address
		throw(std::runtime_error("failed to bind socket"));
	if (listen(serSocketFd, SOMAXCONN) == -1) //-> listen for incoming connections and making the socket a passive socket
		throw(std::runtime_error("listen() failed"));
	newPoll.fd = serSocketFd; //-> add the server socket to the pollfd
	newPoll.events = POLLIN; //-> set the event to POLLIN for reading data
	newPoll.revents = 0; //-> set the revents to 0
	fds.push_back(newPoll); //-> add the server socket to the pollfd
}

void Server::serverInit(int port, std::string pass)
{
	this->port = port;
	this->password = pass;
	serSocket(); //-> create the server socket

	std::cout << GRE << "Server <" << serSocketFd << "> Connected" << WHI << std::endl;
	std::cout << "Waiting to accept a connection...\n";

	while (Server::signal == false){ //-> run the server until the signal is received

		if((poll(&fds[0],fds.size(),-1) == -1) && Server::signal == false) //-> wait for an event
			throw(std::runtime_error("poll() faild"));

		for (size_t i = 0; i < fds.size(); i++){ //-> check all file descriptors
			if (fds[i].revents & POLLIN){ //-> check if there is data to read
				if (fds[i].fd == serSocketFd)
					acceptNewClient(); //-> accept new client
				else
					receiveNewData(fds[i].fd); //-> receive new data from a registered client
			}
		}
	}
	closeFds(); //-> close the file descriptors when the server stops
}

void Server::sendCapabilities(int fd)
{
	std::string capMessage = "CAP * LS :multi-prefix sasl\r\n";
	send(fd, capMessage.c_str(), capMessage.size(), 0);
}

void Server::processCapReq(int fd, const std::string& message)
{
	if (message.find("CAP REQ") != std::string::npos){
		std::string capAck = "CAP * ACK :multi-prefix sasl\r\n";
		send(fd, capAck.c_str(), capAck.size(), 0);
	}
}

void Server::validatePassword(int fd, const std::string& message)
{
	if (message.rfind("PASS", 0) == 0)
	{ // Check if message starts with "PASS"
		std::string receivedPassword = message.substr(5); // Extract password
		receivedPassword.erase(0, receivedPassword.find_first_not_of(" \t\r\n")); // Remove leading whitespace
		receivedPassword.erase(receivedPassword.find_last_not_of(" \t\r\n") + 1); // Remove trailing whitespace
		// std::cout << RED <<  "-" << this->password << "-" << std::endl;
		// std::cout << GRE << "-" << receivedPassword << "-" << std::endl;
		if (authenticatedClients[fd] == true)
		{
			send(fd, "462 PASS: You may not register\r\n", 33, 0);
			return ;
		}
		if (receivedPassword.empty())
		{
			send(fd, "461 PASS :Not enough parameters\r\n", 34, 0); // ERR_NEEDMOREPARAMS
			return ;
		}
		if (receivedPassword.compare(this->password) == 0)
		{
			this->markPasswordAccepted(fd);
			return ; // Authentication successful
		} 
		else
        {
			send(fd, "464 :Password incorrect\r\n", 26, 0); // ERR_PASSWDMISMATCH
			return ;
        }
	}
	else
        send(fd, "461 PASS :Not enough parameters\r\n", 34, 0); // ERR_NEEDMOREPARAMS
	return ; // Authentication failed
}

void Server::processNickUser(int fd, const std::string& message)
{
	// NICK command
	if (message.rfind("NICK ", 0) == 0)
	{
		std::string nickname = message.substr(5); // Extract nickname
		nickname.erase(0, nickname.find_first_not_of(" \t\r\n"));
		nickname.erase(nickname.find_last_not_of(" \t\r\n") + 1);
		if (nickname.empty())
		{
            send(fd, "431 :No nickname given\r\n", 24, 0); // ERR_NONICKNAMEGIVEN
			return;
		}
		if (!isValidNickname(nickname))
		{
            std::string errorMsg = "432 " + nickname + " :Erroneous nickname\r\n"; // ERR_ERRONEUSNICKNAME
			send(fd, errorMsg.c_str(), errorMsg.length(), 0);
			return;
		}
		if (nicknameMap.find(nickname) != nicknameMap.end())
		{
 			std::string errorMsg = "433 " + nickname + " :Nickname is already in use\r\n"; // ERR_NICKNAMEINUSE
			send(fd, errorMsg.c_str(), errorMsg.length(), 0);
			return;
		}
		// Update client's nickname
		Client& client = getClient(fd);
		std::string oldNickname = client.getNickname();
		if (!oldNickname.empty())
			nicknameMap.erase(oldNickname); // Remove old nickname from the map
		client.setNickname(nickname);
		nicknameMap[nickname] = fd; // Add the new nickname to the map
		std::string response = ":" + oldNickname + " NICK " + nickname + "\r\n"; // Inform the client of the nickname change
		send(fd, response.c_str(), response.length(), 0);
		std::cout << "Client <" << fd << "> changed nickname to: " << nickname << std::endl;
	}
	else if (message.rfind("USER ", 0) == 0)
	{
		std::cout << "Received USER command: " << message << std::endl;
        // Further processing for USER command can be added here
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

Client& Server::getClient(int fd)
{
	for (std::vector<Client>::iterator it = clients.begin(); it != clients.end(); ++it)
	{
		if (it->getFd() == fd)
			return *it;
	}
	throw std::runtime_error("Client not found");
}

void Server::markPasswordAccepted(int fd)
{
    authenticatedClients[fd] = true;
}

bool Server::isValidNickname(const std::string& nickname)
{
	// Ensure the nickname is not too long or too short
	if (nickname.length() < 1 || nickname.length() > 30) // Adjust max length as per your protocol
		return false;
    // Ensure the first character is not a digit, space, colon, or special character
	char firstChar = nickname[0];
	if (!std::isalpha(firstChar) && firstChar != '[' && firstChar != '{' &&
		firstChar != ']' && firstChar != '}' && firstChar != '\\' &&
		firstChar != '|')
		return false;
	// Ensure no invalid characters are in the nickname
	for (std::string::const_iterator it = nickname.begin(); it != nickname.end(); ++it)
	{
		char c = *it;
		if (!(std::isalnum(c) || c == '[' || c == ']' || c == '{' || c == '}' ||
			c == '\\' || c == '|'))
			return false;
	}
	return true;
}