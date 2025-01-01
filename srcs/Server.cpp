#include "Server.hpp"

Server::Server(int port, std::string password)
{
	this->port = port;
	this->password = password;
	//Configuring and bind server socket
	this->serverSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (serverSocket < 0)
		throw std::runtime_error("failed to create socket");
	memset(&this->serverAddr, 0, sizeof(this->serverAddr));
    this->serverAddr.sin_family = AF_INET;
    this->serverAddr.sin_addr.s_addr = INADDR_ANY;
    this->serverAddr.sin_port = htons(port);
	int en = 1;
	if(setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &en, sizeof(en)) == -1)
		throw(std::runtime_error("faild to set option (SO_REUSEADDR) on socket"));
	if (fcntl(serverSocket, F_SETFL, O_NONBLOCK) < 0)
		throw std::runtime_error("failed to set non-blocking mode");
    if (bind(serverSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0)
		throw std::runtime_error("failed to bind socket to address or port");
	if (listen(serverSocket, SOMAXCONN) < 0)
		throw std::runtime_error("failed to listen");
	// Set up polling for server socket
	struct pollfd serverPollFd;
	serverPollFd.fd = serverSocket;
	serverPollFd.events = POLLIN;
	serverPollFd.revents = 0;
	this->fds.push_back(serverPollFd); //Adding to the poll structure
	// std::cout << fds[0].fd << std::endl;
}

Server::~Server()
{
	//To be determined, close all fds and
}

void Server::acceptNewClient()
{
	//New Client and other structures created
	Client newClient; //
	struct sockaddr_in clientAddr;
	struct pollfd newClientPollfd;

	socklen_t clientLen = sizeof(clientAddr);
	int newClientFd = accept(serverSocket, (struct sockaddr *)&clientAddr, &clientLen);
	if (newClientFd == -1)
		std::cerr << "Error accepting new client" << std::endl;
	if (fcntl(newClientFd, F_SETFL, O_NONBLOCK) < 0)
		throw std::runtime_error("failed to set non-blocking mode");
	
	// Add the new client socket to the fds vector for polling
	newClientPollfd.fd = newClientFd;
	newClientPollfd.events = POLLIN; // Monitor for incoming data from the client
	newClientPollfd.revents = 0;
	fds.push_back(newClientPollfd);

	//Add the clients' data
	newClient.setNickname("Guest");
	newClient.setIPAdd(inet_ntoa(clientAddr.sin_addr));
	newClient.setUsername("Guest");
	clients.push_back(newClient);
	std::cout << "New client connected: " << newClientFd << std::endl;
}

void Server::run()
{
    while (true && !g_exit_flag)
    {
		// Poll for events on the server socket and client sockets
        int pollCount = poll(fds.data(), fds.size(), -1);  // Wait indefinitely for an event
        if (pollCount == -1)
			throw std::runtime_error("Waited too long");
		if (fds[0].revents & POLLIN)
            acceptNewClient();
		// Check for new incoming client connection (server socket)
		for (size_t i = 1; i < fds.size(); ++i)
		{
			if (fds[i].revents & POLLIN)
				receiveNewData(fds[i].fd);
		}
		sendPingToClients();
	}
}

void	Server::receiveNewData(int clientFd)
{
    // This method is responsible for handling the client's interaction in an infinite loop.
    char buffer[1024];
	memset(buffer, 0, sizeof(buffer)); //-> clear the buffer
	ssize_t bytesRead = recv(clientFd, buffer, sizeof(buffer) - 1, 0);
	if (bytesRead > 0)
	{
		buffer[bytesRead] = '\0';  // Null-terminate the received data
		std::string receivedMessage(buffer);
		if (receivedMessage.find("PING ") == 0) // Starts with "PING "
		{
			std::string token = receivedMessage.substr(5); // Extract token after "PING "
			std::string pongMessage = "PONG " + token + "\r\n"; // Construct PONG response
			send(clientFd, pongMessage.c_str(), pongMessage.size(), 0);
			std::cout << "PONG sent to client " << clientFd << std::endl;
		}
		else
		{
			std::cout << "Data has been recieved from: " << clientFd << std::endl;
		}
	}
	else if (bytesRead == 0)
	{
		clearClients(clientFd);
		close(clientFd);
	}
	else
		throw std::runtime_error("Client wasn't able to communicate");
}

void Server::sendPingToClients()
{
	for (size_t i = 0; i < clients.size(); ++i)
	{
		std::stringstream pingMessageStream;
		pingMessageStream << "PING " << time(NULL) << "\r\n"; // Use timestamp as token
		std::string pingMessage = pingMessageStream.str();
		send(clients[i].getFd(), pingMessage.c_str(), pingMessage.size(), 0);
		std::cout << "PING sent to client " << clients[i].getFd() << std::endl;
	}
}

void	Server::closeFds()
{
	for (size_t i = 0; i < clients.size(); i++)
	{ //-> close all the clients
		std::cout << "Client <" << clients[i].getFd() << "> Disconnected" << std::endl;
		close(clients[i].getFd());
	}
	if (this->serverSocket != -1){ //-> close the server socket
		std::cout << "Server <" << this->serverSocket << "> Disconnected" << std::endl;
		close(this->serverSocket);
	}
}

void Server::clearClients(int fd)
{
	for(size_t i = 0; i < fds.size(); i++)
	{ //-> remove the client from the pollfd
		if (fds[i].fd == fd)
		{
			fds.erase(fds.begin() + i);
			break;
		}
	}
	for(size_t i = 0; i < clients.size(); i++)
	{ //-> remove the client from the vector of clients
		if (clients[i].getFd() == fd)
		{
			clients.erase(clients.begin() + i);
			break;
		}
	}
}

/// @brief Authentication is happening here . We are iterating through the client vector and  checking for instances such as where the nickname is repeated
/// @return 
std::string Server::getPass(){return this->password;}


bool Server::isNicknameInUse(const std::string& nickname)
{
    for (std::vector<Client>::iterator it = clients.begin(); it != clients.end(); ++it)
    {
		if (it->getNickname() == nickname)
            return true; // Nickname is already in use
    }
    return false; // Nickname is available
}

// Check for nickname collision
bool Server::isNicknameCollision(const std::string& nickname)
{
    // Reserved nicknames (example: admin, root, server)
    static const char* reservedNicknames[] = {"admin", "root", "server"};
    for (size_t i = 0; i < sizeof(reservedNicknames) / sizeof(reservedNicknames[0]); ++i)
    {
        if (nickname == reservedNicknames[i])
            return true; // Nickname collides with a reserved one
	}
    // Count occurrences of the nickname in the client list
    int count = 0;
    for (std::vector<Client>::iterator it = clients.begin(); it != clients.end(); ++it)
    {
        if (it->getNickname() == nickname)
			 ++count;
    }
	return count > 1; // Collision if more than one client has this nickname
}

// Notify all clients of a nickname change
void Server::notifyClientsOfNicknameChange(Client& updatedClient, const std::string& oldNickname, const std::string& newNickname)
{
    for (std::vector<Client>::iterator it = clients.begin(); it != clients.end(); ++it)
	{
		if (it->getFd() != updatedClient.getFd()) // Skip the client who changed the nickname
		{
			std::string message = ":" + oldNickname + " NICK " + newNickname + "\n";
			send(it->getFd(), message.c_str(), message.length(), 0);
        }
    }
}
