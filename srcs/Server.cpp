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
	if (listen(serverSocket, 10) < 0)
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

void Server::run()
{
    while (true && !g_exit_flag)
    {
		// Poll for events on the server socket and client sockets
        int pollCount = poll(fds.data(), fds.size(), -1);  // Wait indefinitely for an event
        if (pollCount == -1)
           throw std::runtime_error("Waited too long");
		// Check for new incoming client connection (server socket)
		if (fds[0].revents & POLLIN)
		{
			int newClientFd = accept(serverSocket, NULL, NULL);
			if (newClientFd == -1)
			{
				std::cerr << "Error accepting new client" << std::endl;
				continue;
			}
			// Set the new client socket to non-blocking mode
			if (fcntl(newClientFd, F_SETFL, O_NONBLOCK) < 0)
				throw std::runtime_error("failed to set non-blocking mode");
			// Add the new client socket to the fds vector for polling
			struct pollfd newClientPollfd;
			newClientPollfd.fd = newClientFd;
			newClientPollfd.events = POLLIN; // Monitor for incoming data from the client
			newClientPollfd.revents = 0;
			fds.push_back(newClientPollfd);
			Client newClient(newClientFd);
			newClient.setNickname("Guest");
			newClient.setUsername("Guest");
			clients.push_back(newClient);
			std::cout << "New client connected: " << newClientFd << std::endl;
			// handleClient(newClient, 0);
		}
		// Check for client activity (incoming data)
		for (size_t i = 1; i < fds.size(); ++i) // Start from index 1 as index 0 is for the server socket
		{
			if (fds[i].revents & POLLIN)
			{
				for (std::vector<Client>::iterator it = clients.begin(); it != clients.end(); ++it)
				{
					if (it->getFd() == fds[i].fd)
					{
						handleClient(*it, fds[i].fd);
					}
				}
            }
        }
    }
}

void	Server::handleClient(Client& client, int fd)
{
    // This method is responsible for handling the client's interaction in an infinite loop.
    char buffer[512];
    while (true)
    {
		if (fd != 0)
		{	
			ssize_t bytesRead = recv(fd, buffer, sizeof(buffer) - 1, 0);
			if (bytesRead > 0)
			{
				buffer[bytesRead] = '\0';  // Null-terminate the received data
				client.processMessage(buffer, *this);  // Process the message here
				continue;
			}
			else if (bytesRead == 0)
			{
				std::cout << "Client " << client.getFd() << " disconnected." << std::endl;
				break;  // Client has disconnected
			}
			else
			{
				std::cerr << "Error receiving data from client " << client.getFd() << std::endl;
				break;  // Exit loop on error
			}
		}
	}
	// After exiting the loop, remove the client from the server
	for (std::vector<Client>::iterator it = clients.begin(); it != clients.end(); ++it)
	{
		if (it->getFd() == client.getFd())
		{
			clients.erase(it);
			break;
		}
	}
    // Remove the client from the pollfd vector as well
	for (std::vector<struct pollfd>::iterator it = fds.begin(); it != fds.end(); ++it)
	{
		if (it->fd == client.getFd())
		{
			fds.erase(it);
			break;
		}
	}
	close(client.getFd());  // Close the client connection
	std::cout << "Client " << client.getFd() << " removed from server." << std::endl;
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
