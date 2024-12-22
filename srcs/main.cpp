#include "Server.hpp"
#include "Client.hpp"

#include <iostream>
#include <cstdlib> // For std::atoi
#include "Server.hpp"

int main(int ac, char** av)
{
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
        ircServer.run(); // Server is started
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}