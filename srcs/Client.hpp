#pragma once

#include "Server.hpp"
#include "CommandProcessor.hpp"
#include "Channel.hpp"

class CommandProcessor;
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
	    Client();
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
		// Utility
		void processMessage(char buffer[], Server& server);
		void setRegistered(bool isResgistered);
		void setIPAdd(const std::string& ip);
};