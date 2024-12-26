#pragma once 

#include "Client.hpp"
#include "Server.hpp"
#include "Channel.hpp"

class Channel;
class Server;
class Client;

typedef enum commands
{
	NICK,
	USER,
	JOIN,	
};

class CommandProcessor
{
	private:
		CommandProcessor();
		~CommandProcessor();
	public:
		void static centralProcessor(char buffer[], int fd);
};