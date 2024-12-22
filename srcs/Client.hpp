#pragma once

#include "Server.hpp"

class Client //-> class for client
{
    private:
	    int fd; //-> client file descriptor
	    std::string IPadd; //-> client ip address
    public:
	    Client(){}; //-> default constructor
	    int Getf(){return fd;} //-> getter for fd
	    void SetFd(int fd){this->fd = fd;} //-> setter for fd
	    void setIpAdd(std::string ipadd){IPadd = ipadd;} //-> setter for ipadd
};