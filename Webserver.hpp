#ifndef WEBSERVER_HPP
# define WEBSERVER_HPP
# include "webserv.h"
# include "Server.hpp"
# include "Client.hpp"

class Webserver
{
private:
	std::vector<Server> _servers;
	Webserver &operator=(Webserver const &other);
public:
	Webserver();
	Webserver(Webserver const &copy);
	std::vector<Server> &getServers();
	void setServer(Server &server);
	Server &getServer(std::vector<Server>::iterator &it);
	Server &getServer(int i);
	void	run();
	~Webserver();
};

#endif