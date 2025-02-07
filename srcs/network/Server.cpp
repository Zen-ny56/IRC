#include "../../include/Server.hpp"

Server::Server(){serSocketFd = -1;}

void Server::clearClients(int fd)
{ 	//-> clear the clients
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

void	Server::closeFds()
{
	for (size_t i = 0; i < clients.size(); i++){ //-> close all the clients
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
	if (bytes <= 0)
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
		else if (message.rfind("NICK ", 0) == 0)
			processNickUser(fd, message);
		else if (message.rfind("USER ", 0) == 0)
			processUser(fd, message);
		else if (message.find("CAP REQ") != std::string::npos)
			processCapReq(fd, message);
		else if (message.find("QUIT", 0) == 0)
			processQuit(fd, message);
		else if (message.find("JOIN", 0) == 0)
			handleChannel(fd, message);
		else if (message.find("PRIVMSG", 0) == 0)
			processPrivmsg(fd, message);
		else if (message.find("AUTHENTICATE") != std::string::npos)
			processSasl(fd, message);
		else if (message.find("CAP END") != std::string::npos)
			capEnd(fd);
		else
		{	
			if (message.length() > 0)
			{
				std::cout << "\033[?12h"; // Enable blinking cursor
				std::cout << "Typed : Command Unkown. " << message;
    			std::cout << "\033[?12l";
			}
			// std::cout << YEL << "Client <" << fd << "> Data: " << WHI << buff;
		}
	}
}

void Server::disconnectClient(int fd)
{
	clearClients(fd);
	close(fd);
	std::cout << RED << "Client <" << fd <<  "> Disconnected" << std::endl;
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

	// authenticatedClients[incofd] = false;
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

	while (Server::signal == false)
	{ //-> run the server until the signal is received

		if ((poll(&fds[0],fds.size(),-1) == -1) && Server::signal == false) //-> wait for an event
			throw(std::runtime_error("poll() faild"));

		for (size_t i = 0; i < fds.size(); i++){ //-> check all file descriptors
			if (fds[i].revents & POLLIN)
			{ //-> check if there is data to read
				if (fds[i].fd == serSocketFd)
					acceptNewClient(); //-> accept new client
				else
					receiveNewData(fds[i].fd); //-> receive new data from a registered client
			}
		}
	}
	closeFds(); //-> close the file descriptors when the server stops
}

void Server::sendWelcome(int fd, Client& client)
{
	// 1. RPL_WELCOME (001)
	std::string welcomeMsg = std::string(YEL) + ":" + "ircserv" + " 001 " + client.getNickname() + " :Welcome to the IRC Network " + client.getNickname() + "!" + client.getUserName() + "@" + client.getIPadd() + "\r\n";
	send(fd, welcomeMsg.c_str(), welcomeMsg.size(), 0);

	// 2. RPL_YOURHOST (002)
	std::string yourHostMsg = std::string(YEL) + ":" + "ircserv" + " 002 " + client.getNickname() + " :Your host is " + "ircserv" + ", running version 1.0" + "\r\n";
	send(fd, yourHostMsg.c_str(), yourHostMsg.size(), 0);

	// 3. RPL_CREATED (003)
	std::string createdMsg = std::string(YEL) + ":" + "ircserv" + " 003 " + client.getNickname() + " :This server was created on 01 Jan 2020" + "\r\n";
	send(fd, createdMsg.c_str(), createdMsg.size(), 0);

	// 4. RPL_MYINFO (004)
	std::string myInfoMsg = std::string(YEL) + ":" + "ircserv" + " 004 " + client.getNickname() + " " + "IRCserv" + " v1.0 :Welcome to IRC Network" + "\r\n" + std::string(WHI);
	send(fd, myInfoMsg.c_str(), myInfoMsg.size(), 0);
}

std::vector<Client>::iterator Server::getClient(int fd)
{
	for (std::vector<Client>::iterator it = clients.begin(); it != clients.end(); ++it)
	{
		if (it->getFd() == fd)
			return it;
	}
	return(clients.end());
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

std::vector<std::string> Server::splitByDelimiter(const std::string& str, char delimiter)
{
	std::vector<std::string> result;
	std::stringstream ss(str);
	std::string item;
	while (std::getline(ss, item, delimiter))
	{
		item = trim(item);
		if (!item.empty())
			result.push_back(item);
	}
	return result;
}

bool Server::isValidChannelName(const std::string& channelName)
{
	if (channelName.empty() || (channelName[0] != '#' && channelName[0] != '&'))
        return false;
	// Additional validation rules can be added here
	return true;
}

Client& Server::operator[](std::vector<Client>::iterator it)
{
	if (it == clients.end())
		throw std::out_of_range("Iterator out of range for clients vector");
	return *it;
}

// // Helper methods for getting client and checking channels
std::vector<Client>::iterator Server::getClientUsingNickname(const std::string& nickname)
{
	for (std::vector<Client>::iterator it = clients.begin(); it != clients.end(); ++it)
	{
		std::string clientsNick = it->getNickname();
		if (clientsNick.compare(nickname) == 0)
			return it;
	}
	return (clients.end());
}


std::string Server::trim(const std::string& str)
{
    size_t start = str.find_first_not_of("\r\n\t");
    size_t end = str.find_last_not_of("\r\n\t");
    return (start == std::string::npos || end == std::string::npos) ? "" : str.substr(start, end - start + 1);
}
