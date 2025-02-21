#include "../../include/Client.hpp"
#include "../../include/Channel.hpp"

Client::Client()
    : passAuthen(false), userAuthen(false), nickAuthen(false),
      userName("default"), realName("default"), _channel(NULL), fd(-1), IPadd(""), nickName("") {}

Client::~Client() {}

// Getters
int Client::getFd() const { return fd; }

std::string Client::getIPadd() const { return IPadd; }

std::string Client::getUserName() const { return userName; }

bool Client::getPassAuthen() const { return passAuthen; }

bool Client::getUserAuthen() const { return userAuthen; }

bool Client::getNickAuthen() const { return nickAuthen; }

Channel* Client::getChannel() const { return _channel; }

// Setters
void Client::setFd(int fd) { this->fd = fd; }

void Client::setIpAdd(const std::string& IPadd) { this->IPadd = IPadd; }

void Client::setNickname(const std::string& nickName)
{
    this->nickName = nickName;
    this->nickAuthen = true;
}

std::string Client::getNickname() const {return this->nickName; }

void Client::setUserName(const std::string& userName, const std::string& realName)
{
    this->userName = userName;
    this->realName = realName;
    this->userAuthen = true;
}

void Client::setPassAuthen(bool value) { this->passAuthen = value; }

void Client::setUserAuthen(bool value) { this->userAuthen = value; }

void Client::setNickAuthen(bool value) { this->nickAuthen = value; }

void Client::setChannel(Channel* channel) { this->_channel = channel; }

// Utility functions
void Client::write(const std::string& message) const
{
    std::string buff = message + "\r\n";
    if (send(fd, buff.c_str(), buff.size(), 0) <= -1)
        throw std::runtime_error("Error occurred while sending");
}