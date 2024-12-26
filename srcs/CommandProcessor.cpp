#include "CommandProcessor.hpp"

bool    CommandProcessor::authUser(char buffer[], int fd, Server& server, Client& client)
{
	if (strncmp(buffer, "PASS", 4) == 0)
	{
		std::istringstream stream(buffer);
		std::string command, password;
		stream >> command >> password;
		if (password == server.getPass())
		{
			std::cout << "Password has been accepted" << std::endl;
			return (true);
		}
		else
		{
			std::cout << "Wrong Password" << std::endl;
			return (false);
		}
	}
	if (strncmp(buffer, "USER", 4) == 0)  // Note: corrected "5" to "4" for correct comparison
	{
		std::istringstream stream(buffer);  // Create an input stream from the buffer
		std::string command, username;
		stream >> command >> username;
		client.setUsername(username);
		std::cout << "Received USER command, setting username to: " << username << std::endl;
		return true;
	}
}

void    CommandProcessor::centralProcessor(char buffer[], int fd, Server& server, Client& client)
{
}
