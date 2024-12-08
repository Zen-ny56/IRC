#pragma once

class ClientManager
{
    private:
        std::vector<Client> clients;

    public:
        void addClient(int fd);
        void removeClient(int fd);
        void handleClients(const std::vector<int> &activeSockets, CommandProcessor &processor);
};