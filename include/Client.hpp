#pragma once

#include <string>
#include <cstring>
#include <stdexcept>
#include <sys/socket.h>
#include <errno.h>
#include <netinet/in.h>

class Channel; // Forward declaration

class Client
{
private:
    bool passAuthen;
    bool userAuthen;
    bool nickAuthen;
    std::string userName;
    std::string realName;
    Channel* _channel;
    int fd; // Client file descriptor
    std::string IPadd; // Client IP address
    std::string nickName;

public:
    Client(); // Default constructor
    ~Client(); // Destructor

    // Getters
    int getFd() const;
    std::string getIPadd() const;
    std::string s6_addr32(std::string targetNick);
    std::string getUserName() const;
    bool getPassAuthen() const;
    bool getUserAuthen() const;
    bool getNickAuthen() const;
    Channel* getChannel() const;
    void setNickname(const std::string& nickName);
    // Setters
    void setFd(int fd);
    void setIpAdd(const std::string& IPadd);
    void setUserName(const std::string& userName, const std::string& realName);
    void setPassAuthen(bool value = true);
    void setUserAuthen(bool value = true);
    void setNickAuthen(bool value = true);
    void setChannel(Channel* channel);
    std::string getNickname() const;
    // Utility functions
    void write(const std::string& message) const;
};