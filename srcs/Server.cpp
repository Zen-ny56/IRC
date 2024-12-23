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
			std::cout << "New client connected: " << newClientFd << std::endl;
			
		}
		// Check for client activity (incoming data)
		for (size_t i = 1; i < fds.size(); ++i) // Start from index 1 as index 0 is for the server socket
		{
			if (fds[i].revents & POLLIN)
			{
				char buffer[512];
				ssize_t bytesRead = recv(fds[i].fd, buffer, sizeof(buffer) - 1, 0);
				if (bytesRead > 0)
				{
					buffer[bytesRead] = '\0';// Null-terminate the received data
					std::cout << "Received message from client " << fds[i].fd << ": " << buffer << std::endl;
				}
				else if (bytesRead == 0)
				{
					// Client disconnected
					close(fds[i].fd);
					fds.erase(fds.begin() + i);
					std::cout << "Client disconnected: " << fds[i].fd << std::endl;
					--i;// Adjust index after removing an element
				}
    			else
					std::cerr << "Error receiving data from client " << fds[i].fd << std::endl;
            }
        }
    }
}
