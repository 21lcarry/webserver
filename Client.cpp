#include "Client.hpp"

Client::Client(int fd, std::string& ip) : _fd(fd), _ip(ip)
{
	_send = -1;
}

void Client::setRequest(std::string &request)
{
	_request = request;
}

void Client::setResponse(std::string &response)
{
	_response = response;
}

std::string& Client::getResponse()
{
	return (_response);
}

const Request & Client::getRequest() const
{
	//return(_request);
	return(_orequest);
}

const std::string &Client::getRawRequest() const
{
	return _request;
}

void  Client::setRequest(char *request)
{
	_request += std::string(request);
	size_t j, k;
	int flag = 1;
	std::string chunk_end = "0\r\n\r\n";
	size_t i = _request.find("\r\n\r\n");

	if (i != std::string::npos)
	{
		if (_request.find("Content-Length: ") == std::string::npos)
		{
			if (_request.find("Transfer-Encoding: chunked") != std::string::npos)
			{

				j = _request.size();
				k = chunk_end.size();

				while (k > 0)
				{
					j--;
					k--;
					if (j < 0 || _request[j] != chunk_end[k])
					{
						flag = 1;
						break ;
					}
					else
						flag = 0;
				}
			}
			else
				flag = 0;
		}
		else
		{
			size_t	len = std::atoi(_request.substr(_request.find("Content-Length: ") + 16, 10).c_str());

			if (_request.size() >= len + i + 4)
				flag = 0;
			else
				flag = 1;
		}
	}
	if (flag == 0)
	{
		std::cout << _request << std::endl;
		_orequest = Request(_request, _ip);
		_request.clear();
	}	
}

void Client::setResponse(char *response)
{
	_response = std::string(response);
}

Client& Client::operator=(Client const &other)
{
	_fd = other._fd;
	_request = other._request;
	_response = other._request;
	_ip = other._ip;
	return (*this);
}

Client::Client(Client const &copy)
{
	*this = copy;
}

Client::~Client(){}

int Client::getFd()
{
	return(_fd);
}

void Client::setIsSend(int i)
{
	_send = i;
}

int	Client::isSend()
{
	return (_send);
}