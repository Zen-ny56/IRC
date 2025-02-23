#pragma once

#include <algorithm>  // Required for std::find and std::remove
#include <vector>
#include <string>
#include <sstream>
#include <vector>
#include <map>

class Channel
{
typedef std::vector<Client *>::iterator clientIterate;
private:
    std::map<char, bool> modes;
    const std::string channelName;
    std::string key;
    Client* _admin;
    std::vector<Client *> _clients;
    std::string topic;
    bool inviteOnly;
    int max;
    std::vector<int> clientFds;
    std::map<int, bool> _isInvited;
    std::vector<std::string> _isBanned;

public:
    Channel();
    Channel(const std::string& channelName, const std::string& key);
    Channel(const Channel& other);
    Channel& operator=(const Channel& copy);
    ~Channel();    void addClient(Client* client);
    void setKey(const std::string& key);
    void setMax(int max);
    bool getInviteOnly() const;
    Channel* getChannel();
    std::string getNickname(void) const;
    std::string getKey() const;
    int getMax() const;
    bool isFull() const;
    bool isInvited(int fd) const;
    bool isBanned(const std::string& nickName) const;
    bool isInChannel(int fd) const;
    void broadcastToChannel(const std::string& message);
    std::string getTopic() const;
    std::vector<int> listUsers() const;
    void setTopic(const std::string& topic);
    bool isInviteOnly() const;
    std::string getName() const;
    void broadcast(const std::string& message);
    void broadcast(const std::string& message, Client* exclude);
    void removeClient(Client* client);
    void setAdmin(Client* admin);
    Client* getAdmin() const;
};