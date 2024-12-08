#include "ServerManager.hpp"

SocketManager::SocketManager(int port)
{
	//Creating the server
	serverSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (serverSocket < 0)
	{
		perror("Socket creation failed");
		exit(EXIT_FAILURE);
	}
	std::cout << "SocketManager has been created" << std::endl;
	// Set the socket to non-blocking
	modesetNonBlocking(serverSocket);

	//Configure the address of our serverAddr struct, will be binding this later 
	memset(&serverAddr, 0, sizeof(serverAddr)); // Clear memory
	serverAddr.sin_family = AF_INET;            // IPv4 network
	serverAddr.sin_addr.s_addr = INADDR_ANY;    // Accept connections on all available interfaces
	serverAddr.sin_port = htons(port);          // Port number (converted to network byte order)

	// Bind the socket
	if (bind(serverSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0)
	{
		perror("Bind failed");
		exit(EXIT_FAILURE);
	}
	// Start listening
	if (listen(serverSocket, 10) < 0)
	{
		perror("Listen failed");
		exit(EXIT_FAILURE);
	}
	std::cout << "Server listening on port " << port << std::endl;
}

void SocketManager::setNonBlocking(int fd)
{
	if (fcntl(fd, F_SETFL, O_NONBLOCK) < 0)
	{
		perror("Failed to set non-blocking mode");
		exit(EXIT_FAILURE);
	}
}

void SocketManager::pollEvents()
{
	struct pollfd fds[activeSockets.size() + 1];
	fds[0].fd = serverSocket;
	fds[0].events = POLLIN;

	//Setting up client sockets to br checked
	for (size_t i = 0; i < activeSockets.size(); ++i)
	{
		fds[i + 1].fd = activeSockets[i];
		fds[i + 1].events = POLLIN;
    }
	int ret = poll(fds, activeSockets.size() + 1, -1);
    if (ret < 0)
	{
		perror("Poll failed");
		exit(EXIT_FAILURE);
    }
    // Check for new connections
	if (fds[0].revents & POLLIN)
	{
		int clientFd = accept(serverSocket, NULL, NULL);
		if (clientFd >= 0)
		{
			setNonBlocking(clientFd);
			activeSockets.push_back(clientFd);
			std::cout << "New client connected: " << clientFd << std::endl;
        }
    }
    // Check for client activity
	for (size_t i = 1; i <= activeSockets.size(); ++i)
	{
		if (fds[i].revents & POLLIN)
		{
        	// Mark the active client for further processingint
			clientFd = fds[i].fd;
            // (Processing logic is handled by ClientManager)
        }
    }
}

std::vector<int> SocketManager::getActiveSockets() const
{
    return activeSockets;
}

SocketManager:~SocketManager()
{
	std::cout << "SocketManager has been destroyed" << std::endl;
}

