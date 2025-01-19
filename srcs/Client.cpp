#include "Client.hpp"

Client::Client() :passAuthen(false), userAuthen(false), nickAuthen(false), userName("default"), realName("default")
{
    std::cout << RED << "Should only enter here once" << std::endl;
    std::cout << YEL << nickAuthen << std::endl;
}

int Client::getFd(){return fd;}

void Client::setFd(int fd){this->fd = fd;}

void Client::setIpAdd(std::string IPadd){this->IPadd = IPadd;}

void Client::setNickname(std::string nickName)
{
    std::cout << GRE << "We do change the values" << std::endl;
    this->nickName = nickName;
    this->nickAuthen = true;
    std::cout << YEL << this->nickAuthen << std::endl;
 }

void Client::setUserName(std::string userName, std::string realName)
{
    this->userName = userName;
    this->realName = realName;
    this->userAuthen = true;
}

void Client::setPassAuthen(){this->passAuthen = true;}

bool Client::getUserAuthen(){return this->userAuthen;}

bool Client::getNickAuthen(){return this->nickAuthen;}

bool Client::getPassAuthen(){return this->passAuthen;}

std::string Client::getNickname(){return this->nickName;}