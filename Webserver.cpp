#include "Webserver.hpp"

Webserver::Webserver()
{
}

Webserver::~Webserver(){}

void Webserver::setServer(Server &server)
{
	_servers.push_back(server);
}

std::vector<Server>& Webserver::getServers()
{
	return (_servers);
}

Webserver::Webserver(Webserver const &copy)
{
}

Webserver& Webserver::operator=(Webserver const &other)
{
	return (*this);
}

void Webserver::run()
{
	fd_set readSet, writeSet;
	int sdTmp, rc, maxSd, preMaxSd;
	struct timeval timeout;

	preMaxSd = 0;
	for (std::vector<Server>::iterator it = _servers.begin();
		 it != _servers.end(); ++it)
	{
		int tmp;
		it->listenSock(32);
		tmp = it->getFd();
		if (preMaxSd < tmp)
			preMaxSd = tmp;
	}
	while(1)
	{
		timeout.tv_sec = 2;
		timeout.tv_usec = 0;
		FD_ZERO(&readSet);
		FD_ZERO(&writeSet);
		maxSd = preMaxSd;
		/*****************************************************************************
		 * Проходим по всем серверам и клиентам, и добавляем их фд в множества       *
		 *****************************************************************************/
		for (std::vector<Server>::iterator servIt = _servers.begin();
			 servIt != _servers.end(); ++servIt)
		{
			FD_SET(servIt->getFd(), &readSet);
			for (std::vector<Client>::iterator clientIt = servIt->getClients().begin();
				 clientIt != servIt->getClients().end(); ++clientIt)
			{
				sdTmp = clientIt->getFd();
				FD_SET(sdTmp, &readSet);
				FD_SET(sdTmp, &writeSet); //нужны проверки отправлено/получено ли сообщение
				if (sdTmp > maxSd)
					maxSd = sdTmp;
			}
		}
		rc = select(maxSd + 1, &readSet, &writeSet, NULL, &timeout);
		if (rc < 0)
		{
			perror("select() failed");
			exit(EXIT_FAILURE);
		}
		if (rc == 0)
			continue ;
		/*****************************************************************************
		 * Принимаем входящие соединения                                             *
		 *****************************************************************************/
		for (std::vector<Server>::iterator servIt = _servers.begin();
			 servIt != _servers.end(); ++servIt)
		{
			if (FD_ISSET(servIt->getFd(), &readSet))
			{
				servIt->connection();
				//continue ;
			}
		}
		/*****************************************************************************
		 * Читаем с клиентских сокетов                                               *
		 *****************************************************************************/
		for (std::vector<Server>::iterator servIt = _servers.begin();
			 servIt != _servers.end(); ++servIt)
		{
			for (std::vector<Client>::iterator clientIt = servIt->getClients().begin();
				 clientIt != servIt->getClients().end(); ++clientIt)
			{
				sdTmp = clientIt->getFd();
				if (FD_ISSET(sdTmp, &readSet))
				{
					rc = servIt->reciveRequest(*clientIt);
					/*****************************************************************************
		 			* Ошибка чтения из сокета                                                    *
		 			*****************************************************************************/
					if (rc < 0)
					{
						FD_CLR(sdTmp, &readSet);
						servIt->getClients().erase(clientIt);
						break ;
					}
					/*****************************************************************************
		 			* Клиент прервал соединение                                                  *
		 			*****************************************************************************/
					if (rc == 0)
					{
						FD_CLR(sdTmp, &readSet);
						servIt->getClients().erase(clientIt);
						break ;
					}
				}
				/*****************************************************************************
		 		* Пишем в клиентские сокеты                                                  *
		 		*****************************************************************************/
				if (FD_ISSET(sdTmp, &writeSet) && clientIt->isSend() == 0)
				{
					rc = servIt->sendResponse(*clientIt);
					if (rc < 0)
					{
						FD_CLR(sdTmp, &writeSet);
						break ;
					}
				}
			}
		}
	}
}