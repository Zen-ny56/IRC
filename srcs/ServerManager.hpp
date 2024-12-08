#pragma once

#include <vector>
// #include <string>
#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>

class ServerManager
{
    private:
        int serverSocket;
        std::vector<int> activeSockets;
        struct sockaddr_in serverAddr;
        void setNonBlocking(int fd);
        struct pollfd { int fd[];};

    public:
        SocketManager(int port);
        ~SocketManager();
        void pollEvents();
        std::vector<int> getActiveSockets() const;
};