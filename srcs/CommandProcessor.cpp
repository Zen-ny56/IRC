#include "CommandProcessor.hpp"

std::string CommandProcessor::joinUsernames(const std::string& username1, const std::string& username2, const std::string& username3, const std::string& username4)
{
	// Check if any of the usernames are empty
    if (username1.empty() || username2.empty() || username3.empty() || username4.empty()){
        std::string error = "error";
		return error;
    }
	// Concatenate the usernames with spaces
	std::ostringstream result;
	result << username1 << " " << username2 << " " << username3 << " " << username4;
	return result.str();
}

void	CommandProcessor::authUser(char buffer[], int fd, Server& server, Client& client)
{
	if (strncmp(buffer, "PASS", 4) == 0)
	{
		std::istringstream stream(buffer);
		std::string command, password;
		stream >> command >> password;
		// Check if the client is already authenticated
		if (client.isAuthenticated())
		{
			const char* errorMessage = "462 :You may not reauthenticate.\n";
			send(fd, errorMessage, strlen(errorMessage), 0);
			return; // Exit early
		}
		// Check if the password parameter is provided
		if (password.empty())
		{
			const char* errorMessage = "461 :Not enough parameters for PASS command.\n";
			send(fd, errorMessage, strlen(errorMessage), 0);
			return; // Exit early
		}
		// Validate the password
		if (password == server.getPass())
		{
			client.authenticate();
			const char* successMessage = "Authentication successful.\n";
			send(fd, successMessage, strlen(successMessage), 0);
		}
		else
		{
			const char* errorMessage = "464 :Password incorrect.\n";
			send(fd, errorMessage, strlen(errorMessage), 0);
		}
	}
	if (strncmp(buffer, "NICK", 4) == 0)
	{
		if (client.isAuthenticated() == 1)
		{
			const char* errorMessage = "Password has not been entered yet.\n";
			send(fd, errorMessage, strlen(errorMessage), 0);	
		}
		std::istringstream stream(buffer);
		std::string command, nickname;
		stream >> command >> nickname;
		if (nickname.empty() || nickname[0] == '#' || nickname[0] == ':' || isspace(nickname[0]))
		{
			const char* errorMessage = "432 :Erroneous nickname\n";
			send(fd, errorMessage, strlen(errorMessage), 0);
			return; // Exit early if validation fails
		}
		if (server.isNicknameInUse(nickname))
		{
			const char* errorMessage = "433 :Nickname is already in use\n";
			send(fd, errorMessage, strlen(errorMessage), 0);
			return; // Exit early
		}
		// Check for nickname collision
		if (server.isNicknameCollision(nickname))
		{
			const char* errorMessage = "436 :Nickname collision detected\n";
			send(fd, errorMessage, strlen(errorMessage), 0);
			return; // Exit early
		}
		// If valid, set the nickname and notify other clients
		if (client.getNickname() != "nickname")
		{
			const std::string oldNickname = client.getNickname();
			client.setNickname(nickname);
			// Notify this client
			const std::string successMessage = ":" + oldNickname + " NICK " + nickname + "\n";
			send(fd, successMessage.c_str(), successMessage.length(), 0);
			// Notify other clients about the nickname change
			server.notifyClientsOfNicknameChange(client, oldNickname, nickname);
			return;
		}
		client.setNickname(nickname);
		const char* successMessage = "Nickname has been set.\n";
        send(fd, successMessage, strlen(successMessage), 0);
		return;
	}
	if (strncmp(buffer, "USER", 4) == 0)  // Note: corrected "5" to "4" for comrrect comparison
	{
		if (client.isAuthenticated() == 0)
		{
			const char* errorMessage = "461 :Password has not been entered yet.\n";
			send(fd, errorMessage, strlen(errorMessage), 0);
			return; // Exit early
		}

		// Check if a nickname has been set
		if (client.getNickname().empty() || client.getNickname() == "Error: nickname")
		{
			const char* errorMessage = "461 :Nickname hasn't been entered yet.\n";
			send(fd, errorMessage, strlen(errorMessage), 0);
			return; // Exit early
		}
		// Check if the user is already registered
		if (client.isRegistered())
		{
			const char* errorMessage = "462 :You may not reregister\n";
			send(fd, errorMessage, strlen(errorMessage), 0);
			return; // Exit early
		}
		std::istringstream stream(buffer);  // Create an input stream from the buffer
		std::string command, username1, username2, username3, username4;
		stream >> command >> username1 >> username2 >> username3 >> username4;
		if (username1.empty() || username2.empty() || username3.empty() || username4.empty())
    	{
        	const char* errorMessage = "461 :Not enough parameters for USER command\n";
			send(fd, errorMessage, strlen(errorMessage), 0);
			return; // Exit early
		}
		// Join the username components
		std::string fullUsername = joinUsernames(username1, username2, username3, username4);
		// Set the username and mark the client as registered
		client.setUsername(fullUsername);
		client.setRegistered(true);
		}
}

void    CommandProcessor::centralProcessor(char buffer[], int fd, Server& server, Client& client)
{
	if (client.isRegistered() == false)
		authUser(buffer, fd, server, client);
}
