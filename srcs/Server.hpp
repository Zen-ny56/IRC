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
#include <csignal>
#include <sstream>
#include <ctime>   // For the time() function
#include "Client.hpp"
#include <string.h>// For memset

extern volatile sig_atomic_t g_exit_flag;

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
        void closeFds(); //-> close file descriptors
        void clearClients(int fd); //-> clear clients
        std::string getPass();
        void    run();
        bool isNicknameInUse(const std::string& nickname);
        bool isNicknameCollision(const std::string& nickname);
        void handleClient(Client& client, int a);
        void notifyClientsOfNicknameChange(Client& updatedClient, const std::string& oldNickname, const std::string& newNickname);
        void acceptNewClient();
        void receiveNewData(int clientFd);
        void sendPingToClients();
};