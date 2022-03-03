#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <netinet/in.h> 
#include <fcntl.h>

int main()
{
	int sd;
	struct sockaddr_in addr;
	int len_addr = sizeof(addr);
	const int port = 8080;
	int new_socket;
	memset((char*)&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY); //on any ip-address of this machine (just NULL - mask 0.0.0.0);
	addr.sin_port = htons(port); // on port 8080;
	std::string header = "HTTP/1.1 200 OK\nContent-Type: text/html\nContent-Length: ";
	std::string s_from_file;
	std::ifstream fin;
	std::stringstream ss;
	fin.open("info.html");
	ss << fin.rdbuf();
	s_from_file.append(ss.str());
	std::string file_len = std::to_string(s_from_file.length());
	header.append(file_len + "\n\n");
	header.append(s_from_file);
	char *hello = strdup(header.c_str());
	if ((sd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		perror("cannot create socket");
		exit(EXIT_FAILURE);
	}
	int on = 1;
	int rc = setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, (char *)&on, sizeof(on));
	if (rc < 0)
	{
		perror("setsockopt() failed");
		close(sd);
		exit(1);
	}
	rc = fcntl(sd, F_GETFL);
	fcntl(sd, F_SETFL, rc | O_NONBLOCK);
	if (bind(sd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
	{
		perror("bind error");
		return 0;
	}
	
	if (listen(sd, 10))
	{
		perror("Error listen");
		exit(EXIT_FAILURE);
	}
	fd_set readfds, writefds;
	int max_sd;
	int fd;
	int max_clients = 100;
	struct timeval timeout;
	timeout.tv_sec = 3 * 60;
	timeout.tv_sec = 0;
	fd_set readfds, writefds;
	max_sd = sd;
	FD_ZERO(&readfds);
	FD_ZERO(&writefds);
	FD_SET(sd, &readfds);
	int desc_ready;
	while(1)
	{
		memcpy(&writefds, &readfds, sizeof(readfds));
		printf("Waiting on select()...\n");
		rc = select(max_sd + 1, &writefds, NULL, NULL, &timeout);
		if (rc < 0)
		{
			perror(" select() failed");
			break ;
		}
		if (rc == 0)
		{
			printf(" select() timed out. End program.\n");
		}
		desc_ready = rc;
		for (int i = 0; i <= max_sd && desc_ready > 0; ++i)
		{
			if (FD_ISSET(i, &readfds))
			{
				desc_ready = -1;
				if(i == sd)
				{
					printf("Listening sockets is readable\n");
				}
			}
		}
		printf("\n------------------Waiting for new connection------------------------\n");
		if ((new_socket = accept(sd, (struct sockaddr *)&addr, (socklen_t *)&len_addr)) < 0)
		{
			perror("Error accept");
			exit(EXIT_FAILURE);
		}
		char buffer[30000] = {0};
		read(new_socket, buffer, 30000);
		printf("%s\n", buffer);
		printf("--------------------Sending hello------------------------------------\n");
		std::cout << hello << "\n";
		write(new_socket, hello, strlen(hello));
		close(new_socket);
	}
	return 0;	
}
