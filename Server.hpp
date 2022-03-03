#ifndef SERVER_HPP
# define SERVER_HPP
# include "webserv.h"
# include "Client.hpp"
# include "ServerConf.hpp"

class Server

{
private:
/*
	std::string _serverName;
	std::string _allowMethods;
	std::string _redirection;
	std::string _redirectionStatus;
	std::string _root;
	std::string _defaultErrorPage;
	size_t		_maxBodySize;*/

	ServerConf _config;
	int _opt;
	int _addrLen;
	struct sockaddr_in _addr;
	int	_serverFd;
	std::vector<Client> _clients;

	ServerConf &_getConf(const Request &request);

	Server& operator=(Server const &other);
	Server();
public:
	Server(ServerConf &config);
	Server(Server const &copy);
	~Server();
	
	int t;

	void	listenSock(int backlog);
	int		getAddrLen();
	int		getFd();
	void	connection();
	int		sendResponse(Client &client);
	int		reciveRequest(Client &client);
	std::vector<Client>& getClients();
	
};

#endif