#pragma once

#include <string>
#include <stdexcept>
#include <sys/socket.h>

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
    std::string getNickname() const;
    std::string getUserName() const;
    bool getPassAuthen() const;
    bool getUserAuthen() const;
    bool getNickAuthen() const;
    Channel* getChannel() const;

    // Setters
    void setFd(int fd);
    void setIpAdd(const std::string& IPadd);
    void setNickname(const std::string& nickName);
    void setUserName(const std::string& userName, const std::string& realName);
    void setPassAuthen(bool value = true);
    void setUserAuthen(bool value = true);
    void setNickAuthen(bool value = true);
    void setChannel(Channel* channel);

    // Utility functions
    void write(const std::string& message) const;
};