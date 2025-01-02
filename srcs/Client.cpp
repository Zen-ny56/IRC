#include "Client.hpp"
#include <unistd.h> // For close()

Client::Client()
{
	this->authenticated = false;
}

Client::~Client(){close(fd);}

int Client::getFd() { return this->fd;}
std::string Client::getNickname() const { return nickname; }
std::string Client::getUsername() const { return username; }
bool Client::isAuthenticated() const { return authenticated; }


void Client::setNickname(const std::string& nick) { nickname = nick; }
void Client::setUsername(const std::string& user) { username = user; }
void Client::setIPAdd(const std::string& ip) {IPadd = ip;}
void Client::authenticate() { authenticated = true;}
void Client::setFd(int newFd){this->fd = newFd;}
void Client::processMessage(char buffer[], Server& server)
{
	(void)server;
	std::string message(buffer);
	if (message.find("PING ") == 0) // If it's a PING message
	{
		std::string token = message.substr(5); // Extract the token
		std::string pongMessage = "PONG " + token + "\r\n";
		send(fd, message.c_str(), message.size(), 0);
		// std::cout << "Responded with PONG to server: " << pongMessage << std::endl;
	}
}

bool Client::isRegistered() const
{
	return registered;
}

void Client::setRegistered(bool boolean){registered = boolean;}