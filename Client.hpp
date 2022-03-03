#ifndef CLIENT_HPP
# define CLIENT_HPP
# include "webserv.h"
# include "Request.hpp"

class Client
{
private:
	int _fd;
	std::string _response;
	std::string _request;
	Request 	_orequest;
	std::string _ip;
	
	int	_send;
	Client();
public:
	Client(int fd, std::string& ip);
	Client(Client const &copy);
	Client& operator=(Client const &other);
	~Client();

	void	setResponse(std::string &response);
	void	setResponse(char *response);
	void	setRequest(std::string &request);
	void	setRequest(char *request);
	void	setIsSend(int i);
	int		isSend();
	//сделать обработку запросов
	std::string &getResponse();
	const Request &getRequest() const;
	const std::string &getRawRequest() const;
	int getFd();
};

#endif