#pragma once

#include <iostream>
#include <vector> //-> for vector
#include <sys/socket.h> //-> for socket()
#include <sys/types.h> //-> for socket()
#include <netinet/in.h> //-> for sockaddr_in
#include <fcntl.h> //-> for fcntl()
#include <unistd.h> //-> for close()
#include <arpa/inet.h> //-> for inet_ntoa()
#include <poll.h>
#include <csignal
#include "Client.hpp"

class Client;

class Server
{
    private:
        int serverSocket;
        int            port;
        std::string password;
        // static bool signal;
        struct sockaddr_in serverAddr;
        std::vector<Client> clients;
        std::vector<struct pollfd> fds;

    public:
        Server(int port, std::string password);
        ~Server();
        void    run();
};