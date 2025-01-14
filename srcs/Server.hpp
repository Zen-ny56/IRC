#pragma once

#include <iostream>
#include <vector> //-> for vector
#include <sys/socket.h> //-> for socket()
#include <sys/types.h> //-> for socket()
#include <netinet/in.h> //-> for sockaddr_in
#include <fcntl.h> //-> for fcntl()
#include <unistd.h> //-> for close()
#include <arpa/inet.h> //-> for inet_ntoa()
#include <poll.h> //-> for poll()
#include <csignal> //-> for signal()
#include "Client.hpp" //-> for the class Client
//-------------------------------------------------------//
#define RED "\e[1;31m" //-> for red color
#define WHI "\e[0;37m" //-> for white color
#define GRE "\e[1;32m" //-> for green color
#define YEL "\e[1;33m" //-> for yellow color

class Client;

class Server //-> class for server
{
    private:
        int serSocketFd; //-> server socket file descriptor
        static bool signal; //-> static boolean for signal
        int port; //-> server port
        std::string password;
        std::vector<Client> clients; //-> vector of clients
        std::vector<struct pollfd> fds; //-> vector of pollfd
    public:
        Server(); //-> default constructor
        void serverInit(int port, std::string pass); //-> server initialization
        void serSocket(); //-> server socket creation
        void acceptNewClient(); //-> accept new client
        void receiveNewData(int fd); //-> receive new data from a registered client
        static void signalHandler(int signum); //-> signal handler
        void closeFds(); //-> close file descriptors
        void clearClients(int fd); //-> clear clients
        void sendCapabilities(int fd);
        void processCapReq(int fd, const std::string& message);
        bool validatePassword(int fd, const std::string& message);
        void processNickUser(int fd, const std::string& message);
        void processSasl(int fd, const std::string& message);
        void capEnd(int fd);
        Client& getClient(int fd);
};
