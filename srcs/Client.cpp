#include "Client.hpp"
#include <unistd.h> // For close()

Client::Client(int fd) : fd(fd), authenticated(false)
{
}

Client::~Client(){close(fd);}

int Client::getFd() { return fd;}
std::string Client::getNickname() const { return nickname; }
std::string Client::getUsername() const { return username; }
bool Client::isAuthenticated() const { return authenticated; }

void Client::setNickname(const std::string& nick) { nickname = nick; }
void Client::setUsername(const std::string& user) { username = user; }
void Client::authenticate() { authenticated = true; }

void Client::sendMessage(const std::string& message) const {
    send(fd, message.c_str(), message.size(), 0);
}

void Client::processMessage(char buffer[], Server& server)
{
    CommandProcessor::centralProcessor(buffer, this->fd, server, *this);  // Pass 'this' (the current Client object) by reference
}

bool Client::isRegistered() const
{
    return registered;
}

void Client::setRegistered(bool boolean){registered = boolean;}