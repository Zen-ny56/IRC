#include "Server.hpp"
#include "Client.hpp"

#include <iostream>
#include <cstdlib> // For std::atoi
#include "Server.hpp"

volatile sig_atomic_t g_exit_flag = 0;

void signalHandler(int signum)
{
	std::cout << "\nInterrupt signal (" << signum << ") received. Preparing to exit...\n";
	g_exit_flag = 1;
	exit(EXIT_SUCCESS);
}

int main(int ac, char** av)
{
	std::signal(SIGINT, signalHandler); // Handle Ctrl+C
	// Check command-line arguments
	if (ac != 3) {
		std::cerr << "Usage: ./ircserv <port> <password>" << std::endl;
		return 1;
	}
	// Parse port and password
	int port = std::atoi(av[1]);
	std::string password = av[2];
	if (port < 1024 || port > 65535)
	{
		std::cerr << "Error: Invalid port number." << std::endl;
		return 1;     
	}
	try
	{
		Server ircServer(port, password); //Server intialised
		std::cout << "Pray" << std::endl;
		ircServer.run(); // Server is started
	} catch (const std::exception& e) {
		std::cerr << "Error: " << e.what() << std::endl;
		return 1;
	}
	return 0;
}