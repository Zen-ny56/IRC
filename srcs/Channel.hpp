#pragma once 

#include "Client.hpp"
#include "Server.hpp"

class Channel
{
	private:
		// std::map<char, bool> modes;
		std::vector<int> clientFds;
		std::string topic;
	public:
		Channel(const std::string& channelName);
		~Channel();
		void addClient(int fd);
};