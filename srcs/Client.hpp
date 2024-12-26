#pragma once

#include "Server.hpp"

class Client //-> class for client
{
    private:
	    int fd; //-> client file descriptor
	    std::string nickname;
		std::string username;
		std::string hostname;
		bool 	authenticated;
    public:
	    Client(int fd);
		~Client();
		
		int getFd();
		std::string getNickname() const;
		std::string getUsername() const;
		bool isAuthenticated() const;
		// Setters
		void setNickname(const std::string& nick);
		void setUsername(const std::string& user);
		void authenticate();
		// Utility
    	void sendMessage(const std::string& message) const;
		void processMessage(char buffer[]);
};