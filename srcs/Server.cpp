#include "Server.hpp"

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
	if(bytes <= 0){ //-> check if the client disconnected
		std::cout << RED << "Client <" << fd << "> Disconnected" << WHI << std::endl;
		clearClients(fd); //-> clear the client
		close(fd); //-> close the client socket
	}

	else{ //-> print the received data
		buff[bytes] = '\0';
		std::cout << YEL << "Client <" << fd << "> Data: " << WHI << buff;
		//here you can add your code to process the received data: parse, check, authenticate, handle the command, etc...
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

void Server::serverInit()
{
	this->port = 4444;
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