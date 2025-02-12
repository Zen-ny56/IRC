#include "../include/Client.hpp"
#include "../include/Server.hpp"

bool isValidPort(const std::string &port_str, int &port);
bool parseArguments(int ac, char **av, int &port, std::string &pass);

int main(int ac, char **av)
{
    int port;
    std::string pass;
    
    if (!parseArguments(ac, av, port, pass))
        return 1;

    Server ser;
    std::cout << "---- SERVER ----" << std::endl;
    try
    {	signal(SIGINT, Server::signalHandler);  // Handle Ctrl+C
        signal(SIGQUIT, Server::signalHandler); // Handle Ctrl+"\"
        ser.serverInit(port, pass); /*Initialize the server*/
	}
    catch (const std::exception &e)
    {	ser.closeFds(); // Close open file descriptors
        std::cerr << e.what() << std::endl;
	}
    std::cout << "The Server Closed!" << std::endl;
}


bool isValidPort(const std::string &port_str, int &port)
{
    if (port_str.empty())
        return false;
    for (size_t i = 0; i < port_str.length(); i++)
    {
        if (!isdigit(port_str[i]))
            return false;
    }
    port = std::atoi(port_str.c_str());
    return (port > 1024 && port <= 65535);
}
//check for empty port string
//check for anything that isnt a digit
// convert the end value to int with Atoi and set with int &port
//Values for port being > 1024 and <= 65535 (need to check)

bool parseArguments(int ac, char **av, int &port, std::string &pass)
{
    if (ac != 3)
    {
        std::cerr << RED << "Error: Usage: ./ircserv <port> <password>" << std::endl << EN;
		return false;
    }

    std::string port_str = av[1];
    pass = av[2];

    if (!isValidPort(port_str, port))
    {
        std::cerr << RED << "Error: Invalid port. Port must be numeric and between 1025 and 65535." << std::endl << EN;
        return false;
    }

    if (pass.empty())
    {
        std::cerr << RED << "Error: Password cannot be empty." << std::endl << EN;
        return false;
    }


    return true;
}
//Check for arg count
//Check port Value with function isValidPort above
//check if pass is empty, without checking only num
// because pass CAN contain other chars :)
//------------------------------------------------
// Parsing checks passed working:
// ./ircserv	Error: Usage message
// ./ircserv 12345	Error: Usage message
// ./ircserv abc pass	Error: Invalid port
// ./ircserv 70000 pass	Error: Invalid port
// ./ircserv 1023 pass	Error: Invalid port
// ./ircserv 1234 ""	Error: Password cannot be empty
// ./ircserv 6667 pass	---- SERVER ---- (Starts the server)
