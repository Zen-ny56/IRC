#pragma once 

#include "Client.hpp"
#include "Server.hpp"
#include "Channel.hpp"
#include <sstream>

class Channel;
class Server;
class Client;

typedef enum commands
{
	NICK,
	USER,
	PASS,
};

class CommandProcessor
{
	private:
		CommandProcessor();
		~CommandProcessor();
		static bool authUser(char buffer[], int fd, Server& server, Client& client);
		static std::string joinUsernames(const std::string& username1, const std::string& username2, const std::string& username3, const std::string& username4);
	public:
		static void centralProcessor(char buffer[], int fd, Server& server, Client& client);
};