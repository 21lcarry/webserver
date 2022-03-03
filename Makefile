NAME =	webserv

SRCS =	main.cpp\
		Server.cpp\
		Client.cpp\
		Webserver.cpp\
		ServerConf.cpp\
		ParserConf.cpp\
		Request.cpp\
		Response.cpp\
		CGI.cpp

COMP = clang++ -g -fsanitize=address  -std=c++98

OBJS = $(SRCS:.cpp=.o)

all:	$(NAME)

$(NAME):	$(OBJS)
			$(COMP) $(OBJS) -o $(NAME)

clean:
			rm -f $(OBJS)

fclean:		clean
			rm -f $(NAME)

re:			fclean all

.PHONY:	re fclean clean all