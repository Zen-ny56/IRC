#pragma once 

#include "Server.hpp"

class Client //-> class for client
{
private:
	int fd; //-> client file descriptor
	std::string IPadd; //-> client ip address
	std::string nickName;
public:
	Client();// Default constructor
	int getFd();// Getter for fd
	void setNickname(std::string nickName);
	void setFd(int fd); //-> setter for fd
	void setIpAdd(std::string ipadd);//-> setter for ipadd
};
