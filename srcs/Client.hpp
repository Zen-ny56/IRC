#pragma once 

#include "Server.hpp"

class Client //-> class for client
{
private:
	int fd; //-> client file descriptor
	std::string IPadd; //-> client ip address
public:
	Client();// Default constructor
	int getFd();// Getter for fd

	void setFd(int fd); //-> setter for fd
	void setIpAdd(std::string ipadd);//-> setter for ipadd
};
