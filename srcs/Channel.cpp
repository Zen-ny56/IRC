#include "Channel.hpp"

Channel::Channel(const std::string& channelName, const std::string& key): channelName(channelName), key(key), topic(""), inviteOnly(false), max(INT_MAX)
{
}

void Channel::addClient(int fd)
{
	clientFds.push_back(fd);
}

void Channel::setKey(const std::string& key){this->key = key;}

void Channel::setMax(int max){this->max = max;}

int Channel::getMax(){return this->max;}

int Channel::isFull()
{
	int c = 0;
	for (std::vector<int>::iterator it = clientsFds.begin(); it != clientFds.end(); ++it)
		c++
	if (channel.getMax() == c)
		return (1);
	return (0);
}

std::string Channel::getKey(){return this->key;}

int Channel::isInviteOnly()
{
	if (this->inviteOnly == false)
		return (1);
	return (0);
}

int Channel::isInvited(int fd)
{
	if (_isInvited.find(fd) != _isInvited.end())
	{
		if (_isInvited[fd] == false)
			return 1;
	}
	return 0;
}

int Channel::isBanned(const std::string& nickName)
{
	for (std::vector<std::string>::iterator it = isBanned.begin(); it != isBanned.end(); ++it)
	{
		if (*it.compare(nickName) == 0)
			return (1);
	}
	return (0);
}

int Client::isInChannel(int fd)
{
	for (std::vector<int>::iterator it = clientFds.begin(); it != clientsFds.end(); ++it)
	{
		if (*it == fd)
			return (1);
	}
	return (0);
}

void Channel::broadcastToChannel(const std::string& joinMessage)
{
	for (std::vector<int>::iterator it = clientFds.begin(); it != clientFds.end(); ++it)
	{
		send(*it, joinMessage.c_str(), joinMessage.size(), 0);
	}
}

std::string Channel::getTopic(){return this->topic;}

void Channel::setTopic(const std::string& topic){this->topic = topic;}

std::vector Channel::listUsers()
{
	std::vector<int> temp;
	for (std::vector<int>::iterator it = clientFds.begin(); it != clientFds.end(); ++it)
	{
		temp.push_back(*it);
	}
	return(temp);
}