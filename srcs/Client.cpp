#include "Client.hpp"

Client::Client(){}

int Client::getFd(){return fd;}

void Client::setFd(int fd){this->fd = fd;}

void Client::setIpAdd(std::string IPadd){this->IPadd = IPadd;}

void Client::setNickname(std::string nickName){this->nickName = nickName;}