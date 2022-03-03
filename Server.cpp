#include "Server.hpp"
#include "Response.hpp"

Server::Server(ServerConf &config) :_config(config)
{
	this->t = 0;
	/****************************************************
	*Creating a server socket with options for streaming*
	*communication                                      *
	*****************************************************/
	_serverFd = socket(AF_INET, SOCK_STREAM, 0);
	if (_serverFd < 0)
	{
		perror("socket() failed");
		exit(EXIT_FAILURE);
	}
	_opt = 1;
	/*****************************************************
	*Setting the option for reusing file descriptor after*
	*closing program                                     *
	******************************************************/
	setsockopt(_serverFd, SOL_SOCKET, SO_REUSEADDR,
			   &_opt, (socklen_t)sizeof(_opt));
	/*****************************************************
	*Setting the info about our socket and bind address  *
	*with this socket fd                                 *
	*****************************************************/
	_addrLen = sizeof(_addr);
	bzero(&_addr, _addrLen);
	_addr.sin_family = AF_INET;
	_addr.sin_addr.s_addr = _config.getListen().begin()->ip;
	_addr.sin_port = htons(_config.getListen().begin()->port);
	if (bind(_serverFd, (struct sockaddr *)&_addr, sizeof(_addr)) < 0)
	{
		std::cout << inet_ntoa(_addr.sin_addr) << ":" << _config.getListen().begin()->port << " " << _addr.sin_addr.s_addr << std::endl;
		perror("bind() failed");
		exit(EXIT_FAILURE);
	}
	int flag = fcntl(_serverFd, F_GETFL);
	/*****************************************************
	*Webserv subject: You can only use fcntl as follow:  *
	*fcntl(fd, F_SETFL, O_NONBLOCK); Any other flags     *
	*are forbidden									     *
	*****************************************************/
	fcntl(_serverFd, F_SETFL, O_NONBLOCK); //fcntl(_serverFd, F_SETFL, flag |  O_NONBLOCK);
	std::cout << "*Server " << inet_ntoa(_addr.sin_addr) << ":" << _config.getListen().begin()->port << " created\n";
}

Server::Server(Server const &copy)
{
	_config = copy._config;
	_opt = copy._opt;
	_addr = copy._addr;
	_addrLen = copy._addrLen;
	_serverFd = copy._serverFd;
	_clients = copy._clients;
	this->t = copy.t;
}

Server::Server(){}

Server::~Server()
{
	_clients.clear();
	// close(_serverFd);
	// std::cout << "*Server has been inactivated and fd: "
	// 		  << _serverFd << " is closed\n";
}

void Server::connection()
{
	/*******************************************************
	*Accept connection with client and reciving them socket*
	*descriptor                                            *
	*******************************************************/
	struct sockaddr_in clientAddr;
	socklen_t addrLen = sizeof(clientAddr);
	bzero(&clientAddr, addrLen);

	int newSocket = accept(_serverFd, (struct sockaddr *)&clientAddr,
						   (socklen_t *)&addrLen);
	if (newSocket < 0)
		perror("Accept failed()");
	setsockopt(newSocket, SOL_SOCKET, SO_NOSIGPIPE, &_opt, (socklen_t)sizeof(_opt));
	fcntl(newSocket, F_SETFL, O_NONBLOCK);
	std::string clientIp = inet_ntoa(clientAddr.sin_addr); 
	_clients.push_back(Client(newSocket, clientIp));
	std::cout << "*New connection with client fd " << newSocket << "\n";
}

void Server::listenSock(int backlog)
{
	if (listen(_serverFd, backlog) < 0)
	{
		perror("listen() failed");
		exit(EXIT_FAILURE);
	}
	std::cout << "Server is listening\n";
}

int Server::getAddrLen()
{
	return (_addrLen);
}

int Server::getFd()
{
	return (_serverFd);
}

std::vector<Client>& Server::getClients()
{
	return (_clients);
}

int Server::reciveRequest(Client &client)
{
	char buff[MAX_INPUT];
	bzero(buff, MAX_INPUT);
	int fd = client.getFd();
	int rc = recv(fd, buff, MAX_INPUT, 0);
	if (rc < 0)
	{
		perror("recv() failed");
		return (-1);
	}
	if (rc == 0)
	{
		std::cout << "Connection closed with fd " << fd << "\n";
		close(fd);
		return (0);
	}
	client.setRequest(buff);
	std::cout << "New request on fd " << client.getFd() << "\n";
	/**********************************/
	std::string filename = "logs/request" + std::to_string(t) + ".txt";
	std::ofstream fout(filename, std::ios_base::out | std::ios_base::trunc);
	fout << client.getRequest().getRaw();
	fout.close();
	/**********************************/
	if (client.getRawRequest().empty())
		client.setIsSend(0);
	return (1);
}

ServerConf &Server::_getConf(const Request &request)
{
	std::map<std::string, ServerConf>::iterator it;
	std::string full_path = request.getPath();
	size_t pos = 0;

	if (full_path.find('.') != std::string::npos)
	{
		size_t start = full_path.find_last_of('/');
		size_t end = full_path.find_last_of('.');
		std::string tmp = full_path.substr(0, start + 1);
		tmp += "*" + full_path.substr(end, full_path.size() - end);
		if ((it = _config.getLocation().find(full_path)) != _config.getLocation().end())
		{
			std::cout << it->first << " - FOUND\n";
			return (it->second);
		}
		while (tmp != "")
		{
			if ((it = _config.getLocation().find(tmp)) != _config.getLocation().end())
			{
				std::cout << it->first << " - FOUND\n";
				return (it->second);
			}
			pos = tmp.find_last_of('/');
			if (pos != std::string::npos)
				tmp = tmp.substr(0, pos);
			else
				break ;
		}
	}
	while (full_path != "")
	{
		if ((it = _config.getLocation().find(full_path)) != _config.getLocation().end())
		{
			std::cout << it->first << " - FOUND\n";
			return (it->second);
		}
		pos = full_path.find_last_of('/');
		if (pos != std::string::npos)
			full_path = full_path.substr(0, pos);
		else
			break ;
	}
	std::cout << full_path << std::endl;
	std::cout << "NOT FOUND\n";
	return _config;
}

int Server::sendResponse(Client &client)
{
	Response res(client.getRequest(), _getConf(client.getRequest()));
	std::string header = res.getHeader();
	header += "\r\n";
	if (res.getHeader() == "")
	{
		std::string strFromFile;
		std::ifstream fin;
		std::stringstream ss;
		fin.open(_config.getErrorPage()[500]);
		ss << fin.rdbuf();
		strFromFile.append(ss.str());
		std::string fileLen = std::to_string(strFromFile.length());
		header += "HTTP/1.1 500 Internal Server Error\nContent-Type: text/html\nContent-Length: ";
		header.append(fileLen + "\n\n");
		header.append(strFromFile);
	}

	std::cout << "\n*Send response\n";
	/****************************/
	std::string filename = "logs/response" + std::to_string(t) + ".txt";
	std::ofstream fout(filename, std::ios_base::out | std::ios_base::trunc);
	fout << header;
	fout.close();
	++t;
	/****************************/
	int rc = send(client.getFd(), header.c_str(), header.size(), SO_NOSIGPIPE);
	if (rc < 0)
	{
		perror ("send() failed");
		return (-1);
	}
	client.setIsSend(1);
	return (0);
}