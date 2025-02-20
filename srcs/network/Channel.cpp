#include "../../include/Server.hpp"
#include "../../include/Channel.hpp"

Channel::Channel() : topic("*") {}

Channel::~Channel() {}

Channel::Channel(const std::string& channelName, const std::string& key)
    : channelName(channelName), key(key), topic("*"), inviteOnly(false), max(INT_MAX) {}

Channel::Channel(const Channel& other)
    : channelName(other.channelName), topic(other.topic), inviteOnly(other.inviteOnly), max(other.max),
      clientFds(other.clientFds), _isInvited(other._isInvited), _isBanned(other._isBanned) {}

Channel& Channel::operator=(const Channel& other)
{
    if (this != &other)
    {
        this->key = other.key;
        this->topic = other.topic;
        this->inviteOnly = other.inviteOnly;
        this->max = other.max;
        this->clientFds = other.clientFds;
        this->_isInvited = other._isInvited;
        this->_isBanned = other._isBanned;
    }
    return *this;
}

void Channel::addClient(Client* client)
{
    _clients.push_back(client);
    clientFds.push_back(client->getFd());
}

void Channel::setKey(const std::string& key) { this->key = key; }

void Channel::setMax(int max) { this->max = max; }

int Channel::getMax() const { return this->max; }

bool Channel::isFull() const
{
    return _clients.size() >= static_cast<size_t>(max);
}

std::string Channel::getKey() const { return this->key; }

bool Channel::isInviteOnly() const { return this->inviteOnly; }

bool Channel::isInvited(int fd) const
{
    return _isInvited.find(fd) != _isInvited.end() && _isInvited.at(fd);
}

bool Channel::isBanned(const std::string& nickName) const
{
    return std::find(_isBanned.begin(), _isBanned.end(), nickName) != _isBanned.end();
}

bool Channel::isInChannel(int fd) const
{
    return std::find(clientFds.begin(), clientFds.end(), fd) != clientFds.end();
}

void Channel::broadcastToChannel(const std::string& message)
{
    for (std::vector<int>::const_iterator it = clientFds.begin(); it != clientFds.end(); ++it)
    {
        send(*it, message.c_str(), message.size(), 0);
    }
}

std::string Channel::getTopic() const { return this->topic; }

void Channel::setTopic(const std::string& topic) { this->topic = topic; }

std::vector<int> Channel::listUsers() const { return clientFds; }

void Channel::broadcast(const std::string& message)
{
    for (std::vector<Client*>::iterator it = _clients.begin(); it != _clients.end(); ++it)
    {
        (*it)->write(message);
    }
}

std::string Channel::getName() const{return channelName;}

void Channel::broadcast(const std::string& message, Client* exclude)
{
    for (std::vector<Client*>::iterator it = _clients.begin(); it != _clients.end(); ++it)
    {
        if (*it != exclude)
        {
            (*it)->write(message);
        }
    }
}

std::vector<std::string> Channel::split(const std::string& str, char delimiter)
{
    std::vector<std::string> tokens;
    std::stringstream ss(str);
    std::string token;

    while (std::getline(ss, token, delimiter))
    {
        tokens.push_back(token);
    }

    return tokens;
}

void Channel::removeClient(Client* client)
{
    std::vector<Client*>::iterator it = std::find(_clients.begin(), _clients.end(), client);
    if (it != _clients.end())
    {
        _clients.erase(it);

        // Correct std::remove usage
        clientFds.erase(std::remove(clientFds.begin(), clientFds.end(), client->getFd()), clientFds.end());
        
        client->setChannel(NULL);

        if (client == _admin && !_clients.empty())
        {
            _admin = _clients.front();
            broadcast(_admin->getNickname() + " is now the admin of the channel.");
        }
    }
}


void Channel::setAdmin(Client* admin) { _admin = admin; }

Client* Channel::getAdmin() const { return _admin; }