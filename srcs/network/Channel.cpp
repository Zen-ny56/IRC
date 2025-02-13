#include "../../include/Channel.hpp"

Channel::Channel(){}

Channel::~Channel(){}

Channel::Channel(const std::string& channelName, const std::string& key): channelName(channelName), key(key), topic(""), inviteOnly(false), max(INT_MAX){}

Channel& Channel::operator=(const Channel& other)
{
	if (this != &other)
	{
		// Assign each member variable
		// this->channelName = other.channelName; // Note: channelName is const, so cannot be reassigned
		this->key = other.key;                 // key is also const and cannot be reassigned
		this->topic = other.topic;
		this->inviteOnly = other.inviteOnly;
		this->max = other.max;
		this->clientFds = other.clientFds;
		this->_isInvited = other._isInvited;
		this->_isBanned = other._isBanned;
	}
	return *this;
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
	for (std::vector<int>::iterator it = clientFds.begin(); it != clientFds.end(); ++it)
		c++;
	if (this->getMax() == c)
		return (1);
	return (0);
}

std::string Channel::getKey(){return this->key;}

int Channel::isInviteOnly()
{
	if (this->inviteOnly == false)
		return (0);
	return (1);
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
	for (std::vector<std::string>::iterator it = _isBanned.begin(); it != _isBanned.end(); ++it)
	{
		if ((*it).compare(nickName) == 0)
			return (1);
	}
	return (0);
}

int Channel::isInChannel(int fd)
{
	for (std::vector<int>::iterator it = clientFds.begin(); it != clientFds.end(); ++it)
	{
		if (*it == fd)
			return (1);
	}
	return (0);
}

void Channel::broadcastToChannel(const std::string& message)
{
	for (std::vector<int>::iterator it = clientFds.begin(); it != clientFds.end(); ++it)
	{
		send(*it, message.c_str(), message.size(), 0);
	}
}

std::string Channel::getTopic(){return this->topic;}

void Channel::setTopic(const std::string& topic){this->topic = topic;}

std::vector<int> Channel::listUsers()
{
	std::vector<int> temp;
	for (std::vector<int>::iterator it = clientFds.begin(); it != clientFds.end(); ++it)
	{
		temp.push_back(*it);
	}
	return(temp);
}

//new functions
void Channel::broadcast(const std::string& message)
{
    clientIterate start = _clients.begin();
	clientIterate end = _clients.end();

	while(start != end)
	{
		(*start)->write(message);
		start++;
	}
}

void Channel::broadcast(const std::string& message, Client* exclude)
{
	clientIterate start = _clients.begin();
    clientIterate end = _clients.end();

    while(start!= end)
    {
        if (*start == exclude)
		{
			start++;
			continue;
		}
        (*start)->write(message);
        start++;
    }
}

void   Channel::removeClient(Client *client)
{
	clientIterate start = _clients.begin();
	clientIterate end = _clients.end();

	while(start!= end)
	{
		if (*start == client)
        {
            _clients.erase(start);
            return;
        }
        start++;
	}
	client->set_channel(NULL);
	if (client == _admin)
    {
        _admin = *(_clients.begin());

        std::string message = client->getNickname() + " is now the admin of the channel ";
    }
}

void   Channel::kick(Client* client, Client* target, const std::string& reason)
{
	std::cout << client->getNickname() << " " << " in " << channelName << " kicked " << target->getNickname() << " for reason of : " << reason;
	this->removeClient(target);
	std::string message = client->getNickname() + " kicked " + target->getNickname() + " from channel ";
	std::cout << message << std::endl;
}
