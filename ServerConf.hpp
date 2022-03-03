#ifndef SERVERCONF_HPP
# define SERVERCONF_HPP
# include "webserv.h"
# define CONF_READ_BUFFER 4096
/*ошибки парсинга*/
# define DEFAULT_CONF_ERROR 2
# define CONF_READ_ERROR 3
# define NO_SERVER_DIRECTIVE 4
# define BRACE_EXPECTED 5
# define DIRECTIVE_OUTSIDE_SERVER 6
# define READ_DIRECTIVE_ERROR 7
/*****************/
# define IS_DEFAULT_CONF 0

typedef struct s_listen {
    unsigned int ip;
    int port;
}t_listen;

class ServerConf
{
    public:
        ServerConf();
        ServerConf(int a) : _null(true) {} //конструктор для создания пустого обьекта с флагом parent->default conf
        ~ServerConf();
        ServerConf(ServerConf const &o);
        ServerConf &operator=(ServerConf const &o);


        void replaceListen(t_listen &o);
        /*geters*/
        const int &getPort();
        ServerConf* getParent();
        const bool getDefaultFlag() const;
        ServerConf &getLocation(std::string &name);
        std::map<std::string, ServerConf> &getLocation();
        const bool &getAutoIndex() const;
        std::string &getRoot();
        std::vector<t_listen> &getListen();
        std::vector<std::string> &getServerName();
        std::map<int, std::string> &getErrorPage();
        size_t &getClientBodyBufferSize();
        std::map<std::string, std::string> &getCgiParam();
        std::string &getCgiPass();
        std::vector<std::string> &getAllowedMethods();
        std::vector<std::string> &getIndex();
        std::string &getAlias();
        bool &getAliasSet();
        const  bool &getAutoIndexSet() const;
        /*seters*/
        void setParent(ServerConf *parent);
        void setRoot(std::string &root);
        void setLocation(std::string name, ServerConf conf);
        void setAutoIndex(bool set);
        bool setListen(unsigned int ip, int port);
        void setServerName(std::vector<std::string> &names);
        void setErrorPage(std::vector<int> &codes, std::string &uri);
        void setClientBodyBufferSize(size_t &value);
        void setCgiParam(std::string &param, std::string &val);
        void setCgiPass(std::string &val);
        void setAllowedMethods(std::vector<std::string> &val);
        void setIndex(std::vector<std::string> &val);
        void setAlias(std::string &val);
    private:
        std::vector<std::string>            _allowed_methods;
        std::map<int, std::string>          _default_page;
        std::vector<t_listen>                 _listen;
		std::string                         _root;
		std::vector<std::string>            _server_name;
		size_t                              _client_body_buffer_size;
		std::map<std::string, std::string>	_cgi_param;
		std::string							_cgi_pass;
		std::map<std::string, ServerConf>   _location;
		std::vector<std::string>			_index;
		bool								_autoindex;
        bool                                _autoindexSet;
		std::string							_alias;
		bool								_aliasSet;
        bool                                _null; // является ли обьект пустым, используется как parent для default conf
        ServerConf*                         _parentPtr;
};

std::ostream	&operator<<(std::ostream &out, ServerConf &server);

#endif