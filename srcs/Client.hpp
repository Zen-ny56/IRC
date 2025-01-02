#pragma once

#include "Server.hpp"
#include "Channel.hpp"

class Channel;
class Server;

class Client //-> class for client
{
    private:
	    int fd; //-> client file descriptor
	    std::string nickname;
		std::string username;
		std::string hostname;
		std::string IPadd;// client IP address
		bool 	authenticated;
		bool	registered;
    public:
	    Client(const std::string& username, const std::string& nickname, int fd, std::string IPadd);
		~Client();
		
		int getFd();
		std::string getNickname() const;
		std::string getUsername() const;
		bool isAuthenticated() const;
		bool isRegistered() const;
		// Setters
		void setNickname(const std::string& nick);
		void setUsername(const std::string& user);
		void authenticate();
		void setFd(int newFd);
		// Utility
		void processMessage(char buffer[], Server& server);
		void setRegistered(bool isResgistered);
		void setIPAdd(const std::string& ip);
		std::string getIPAdd();
};