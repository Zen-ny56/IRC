#pragma once 

#include "Client.hpp"
#include "Server.hpp"

class Channel
{
	private:
		// std::map<char, bool> modes;
		const std::string channelName;
		const std::string key;
		const std::string topic;
		bool inviteOnly;
		int max;
		std::vector<int> clientFds;
		std::map<int, bool> _isInvited;
		std::vector<std::string> isBanned;
	public:
		Channel(const std::string& channelName);
		~Channel();
		void addClient(int fd);
		void setKey(const std::string& key);
		int isInChannel(int fd);
};