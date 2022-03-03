#include "ServerConf.hpp"

ServerConf::ServerConf()
{
    _aliasSet = false;
    _autoindexSet = false;
    _null = false;
    _client_body_buffer_size = 0;
}

ServerConf::~ServerConf() {}

ServerConf::ServerConf(ServerConf const &o)
{
    if (this != &o)
    {
        _allowed_methods = o._allowed_methods;
        _default_page = o._default_page;
        _listen = o._listen;
        _root = o._root;
        _server_name = o._server_name;
        _client_body_buffer_size = o._client_body_buffer_size;
        _cgi_param = o._cgi_param;
        _cgi_pass = o._cgi_pass;
        _location = o._location;
        _index = o._index;
        _autoindex = o._autoindex;
        _alias = o._alias;
        _aliasSet = o._aliasSet;
        _null = o._null;
        _parentPtr = o._parentPtr;
    }
}

ServerConf &ServerConf::operator=( ServerConf const &o) 
{
    if (this != &o)
    {
        _allowed_methods = o._allowed_methods;
        _default_page = o._default_page;
        _listen = o._listen;
        _root = o._root;
        _server_name = o._server_name;
        _client_body_buffer_size = o._client_body_buffer_size;
        _cgi_param = o._cgi_param;
        _cgi_pass = o._cgi_pass;
        _location = o._location;
        _index = o._index;
        _autoindex = o._autoindex;
        _alias = o._alias;
        _aliasSet = o._aliasSet;
        _null = o._null;
        _parentPtr = o._parentPtr;
    }
    return *this;
}

void ServerConf::replaceListen(t_listen &o)
{
    std::vector<t_listen> empty;

    _listen.clear();
    _listen.swap(empty);
    _listen.push_back(o);
}

ServerConf *ServerConf::getParent()
{
    return _parentPtr;
}

void ServerConf::setParent(ServerConf *parent)
{
    _parentPtr = parent;
}

void ServerConf::setLocation(std::string name, ServerConf conf)
{
    _location.insert(std::make_pair(name, conf));
}

void ServerConf::setAutoIndex(bool set)
{
    if (set)
        _autoindex = true;
    else
        _autoindex = false;
    _autoindexSet = true;
}

bool ServerConf::setListen(unsigned int ip, int port)
{
    t_listen add;

    for (std::vector<t_listen>::iterator i = _listen.begin(); i != _listen.end(); ++i)
    {
        if (i->port == port)
            return false;
    }
    add.ip = ip;
    add.port = port;
    _listen.push_back(add);
    return true;
}

void ServerConf::setCgiPass(std::string &val)
{
    _cgi_pass = val;
}

void ServerConf::setRoot(std::string &root)
{
    _root = root;
}

void ServerConf::setServerName(std::vector<std::string> &names)
{
    _server_name = names;
}

void ServerConf::setErrorPage(std::vector<int> &codes, std::string &uri)
{
    for (std::vector<int>::iterator i = codes.begin(); i != codes.end(); ++i)
        _default_page[*i] = uri;
}

void ServerConf::setCgiParam(std::string &param, std::string &val)
{
    _cgi_param[param] = val;
}

void ServerConf::setAllowedMethods(std::vector<std::string> &val)
{
    _allowed_methods = val;
}

void ServerConf::setIndex(std::vector<std::string> &val)
{
    _index.insert(_index.end(), val.begin(), val.end());
}

void ServerConf::setAlias(std::string &val)
{
    _alias = val;
    _aliasSet = true;
}

void ServerConf::setClientBodyBufferSize(size_t &value)
{
    _client_body_buffer_size = value;
}

std::vector<std::string> &ServerConf::getIndex()
{
    return _index;
}

std::vector<std::string> &ServerConf::getAllowedMethods()
{
    return _allowed_methods;
}

std::string &ServerConf::getCgiPass()
{
    return _cgi_pass;
}

std::map<std::string, std::string> &ServerConf::getCgiParam()
{
    return _cgi_param;
}

size_t &ServerConf::getClientBodyBufferSize()
{
    return _client_body_buffer_size;
}

std::map<int, std::string> &ServerConf::getErrorPage()
{
    return _default_page;
}

std::vector<std::string> &ServerConf::getServerName()
{
    return _server_name;
}

std::vector<t_listen> &ServerConf::getListen()
{
    return _listen;
}

std::string &ServerConf::getRoot()
{
    return _root;
}

const bool &ServerConf::getAutoIndexSet() const
{
    return _autoindexSet;
}

const bool &ServerConf::getAutoIndex() const
{
    return _autoindex;
}

const bool ServerConf::getDefaultFlag() const
{
    return _null;
}

ServerConf &ServerConf::getLocation(std::string &name)
{
    return _location[name];
}
std::map<std::string, ServerConf> &ServerConf::getLocation()
{
    return _location;
}

std::string &ServerConf::getAlias()
{
    return _alias;
}

bool &ServerConf::getAliasSet()
{
    return _aliasSet;
}

const int &ServerConf::getPort()
{
    return (_listen[0].port);
}

// stream:
std::ostream	&operator<<(std::ostream &out, ServerConf &server)
{
    out << "----------CONFIG START----------\n\n";
    out << "Allowed methods: \n";
    for (std::vector<std::string>::iterator i = server.getAllowedMethods().begin() ; i != server.getAllowedMethods().end(); ++i)
        out << *i << std::endl;
    out << "Error pages : \n";
    for (std::map<int, std::string>::iterator i = server.getErrorPage().begin(); i != server.getErrorPage().end(); ++i)
        out << i->first << ": " << i->second << std::endl;
    out << "Listen: \n";
    for (std::vector<t_listen>::iterator i = server.getListen().begin(); i != server.getListen().end(); ++i)
    {
        struct in_addr a;
        a.s_addr = i->ip;
        out << inet_ntoa(a) << ":" << i->port << std::endl;
    }
    out << "Root: \n" << server.getRoot() << std::endl;
    out << "Server name: \n";
    for (std::vector<std::string>::iterator i = server.getServerName().begin(); i != server.getServerName().end(); ++i)
        out << *i << std::endl;
    out << "Client body buffer size: \n" << server.getClientBodyBufferSize() << std::endl;
    out << "Cgi param: \n";
    for (std::map<std::string, std::string>::iterator i = server.getCgiParam().begin(); i != server.getCgiParam().end(); ++i)
        out << i->first << ": " << i->second << std::endl;
    out << "Cgi pass: \n" << server.getCgiPass() << std::endl;
    out << "Index: \n";
    for (std::vector<std::string>::iterator i = server.getIndex().begin(); i != server.getIndex().end(); ++i)
        out << *i << std::endl;
    out << "Autoindex: \n" << ((server.getAutoIndex() == true) ? "on\n" : "off\n");
    out << "+++++ Location start +++++\n";
    for (std::map<std::string, ServerConf>::iterator it = server.getLocation().begin(); it != server.getLocation().end(); ++it)
    {
        out << "\nLOCATION: " << it->first << "\n\n";
        out << "\t\tAllowed methods: \n";
        for (std::vector<std::string>::iterator i = it->second.getAllowedMethods().begin() ; i != it->second.getAllowedMethods().end(); ++i)
            out << "\t\t" << *i << std::endl;
        out << "\t\tError pages : \n";
        for (std::map<int, std::string>::iterator i = it->second.getErrorPage().begin(); i != it->second.getErrorPage().end(); ++i)
            out << "\t\t" << i->first << ": " << i->second << std::endl;
        out << "\t\tListen: \n";
        out << "\t\tRoot: \n\t\t" << it->second.getRoot() << std::endl;
        out << "\t\tServer name: \n";
        for (std::vector<std::string>::iterator i = it->second.getServerName().begin(); i != it->second.getServerName().end(); ++i)
            out << "\t\t" << *i << std::endl;
        out << "\t\tClient body buffer size: \n\t\t" << it->second.getClientBodyBufferSize() << std::endl;
        out << "\t\tCgi param: \n";
        for (std::map<std::string, std::string>::iterator i = it->second.getCgiParam().begin(); i != it->second.getCgiParam().end(); ++i)
            out << "\t\t" << i->first << ": " << i->second << std::endl;
        out << "\t\tCgi pass: \n\t\t" << it->second.getCgiPass() << std::endl;
        out << "\t\tIndex: \n";
        for (std::vector<std::string>::iterator i = it->second.getIndex().begin(); i != it->second.getIndex().end(); ++i)
            out << "\t\t" << *i << std::endl;
        out << "\t\tAutoindex: \n" << ((it->second.getAutoIndex() == true) ? "\t\ton\n" : "\t\toff\n");
        out << "\t\tAlias set: \n" << ((it->second.getAliasSet() == true) ? "\t\ttrue\n" : "\t\tfalse\n");
        out << "\t\tAlias: \n\t\t" << it->second.getAlias() << std::endl;
    }
    out << "+++++ Location end +++++\n";
    out << "----------CONFIG END----------\n\n";
    return out;
}