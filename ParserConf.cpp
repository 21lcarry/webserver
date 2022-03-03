#include "ParserConf.hpp"
ParserConf::~ParserConf()
{
	if (_servers_count > 0)
		delete [] _servers;
}

ParserConf::ParserConf(const std::string &path)
{
    _error = 0;
    _initMaps();
    if (!(_setDefaultConf(DEFAULT_CONFIG_PATH)))
        return ;

	std::vector<std::string> file = _readConf(path);
	if (file.empty())
	{
		_error = CONF_READ_ERROR;
		return ;
	}
	if (!(_initConfigArray(file)))
		return ;
	if (!(_parseConfFile(file)))
	{
		std::cerr << "Parsing error. Stop" << std::endl;
		return ;
	}
}

std::vector<ServerConf> ParserConf::getConfigs()
{
	std::vector<ServerConf> result;

	for (int i = 0; i < _servers_count; ++i)
	{
		if (_servers[i].getListen().size() > 1)
		{
			for(std::vector<t_listen>::iterator it = _servers[i].getListen().begin(); it != _servers[i].getListen().end(); ++it)
			{
				ServerConf conf(_servers[i]);
				conf.replaceListen(*it);
				result.push_back(conf);
			}
		}
		else
			result.push_back(_servers[i]);
	}
	return result;
}

bool ParserConf::_parseConfFile(std::vector<std::string> &file)
{
	size_t index = 0, size = file.size();
	int server_index = 1;

	while (index < size) 
	{
		if (file[index] == "server") 
		{
			++index;
			if (file[index] != "{") 
			{
				std::cerr << "webserv: " << "configuration: Error: expected '{' after server directive" << std::endl;
				_error = BRACE_EXPECTED;
				return false;
			}
			++index;
			if (!(_parseConf(index, file, _defaultConf, server_index))) 
				return false;
			else
				++server_index;
		}
		else {
			std::cerr << "webserv: " << "configuration: Error: directive \"" << file[index];
			std::cerr << "\" outside \"server {}\" directive" << std::endl;
			_error = DIRECTIVE_OUTSIDE_SERVER;
			return false;
		}
		++index;
	}
	if (_servers_count > 0)
		for(int i = 0; i < _servers_count; ++i)
			_setDefault(_servers[i]);
	return true;
}

void ParserConf::_setDefault(ServerConf &target, int flag)
{
	ServerConf parent = *target.getParent();

	if (!target.getAutoIndexSet())
		target.setAutoIndex(parent.getAutoIndex());
	if (target.getListen().empty())
		target.getListen().insert(target.getListen().begin(), parent.getListen().begin(), parent.getListen().end());
	if (target.getRoot() == "")
		target.setRoot(parent.getRoot());
	if (target.getServerName().empty())
		target.getServerName().insert(target.getServerName().end(), parent.getServerName().begin(), parent.getServerName().end());
	for (std::map<int, std::string>::iterator i = parent.getErrorPage().begin(); i != parent.getErrorPage().end(); ++i) 
		if (target.getErrorPage().find(i->first) == target.getErrorPage().end())
			target.getErrorPage()[i->first] = i->second;
	if (target.getClientBodyBufferSize() == 0)
		target.setClientBodyBufferSize(parent.getClientBodyBufferSize());
	if (target.getCgiParam().empty())
		for (std::map<std::string, std::string>::iterator i = parent.getCgiParam().begin() ; i != parent.getCgiParam().end(); ++i)
			if (target.getCgiParam().find(i->first) == target.getCgiParam().end())
				target.getCgiParam()[i->first] = i->second;
	if (target.getCgiPass().size() == 0)
		target.setCgiPass(parent.getCgiPass());
	if (target.getAllowedMethods().empty())
		target.getAllowedMethods().insert(target.getAllowedMethods().end(), parent.getAllowedMethods().begin(), parent.getAllowedMethods().end());
	if (target.getIndex().empty())
		target.getIndex().insert(target.getIndex().end(), parent.getIndex().begin(), parent.getIndex().end());
	if (flag == 0)
		for (std::map<std::string, ServerConf>::iterator i = target.getLocation().begin(); i != target.getLocation().end(); ++i)
			_setDefault(i->second, IS_LOCATION_CONF);
}

bool ParserConf::_initConfigArray(std::vector<std::string> &file)
{
	for(std::vector<std::string>::iterator it = file.begin(); it != file.end() - 1; ++it)
	{
		if (*it == "server" && *(it + 1) == "{")
			++_servers_count;
	}
	if (_servers_count > 0)
	{
		_servers = new ServerConf[_servers_count];
		return true;
	}
	std::cerr << "webserv: " << "Сan not init server because no \"server {}\" directive in configuration file" << std::endl;
	_error = NO_SERVER_DIRECTIVE;
	return false;
}

bool ParserConf::_setDefaultConf(const std::string &path)
{
    std::vector<std::string> file;
    std::vector<std::string> raw_conf;
    size_t index = 2;
	int serv = 0;
	ServerConf default_conf(IS_DEFAULT_CONF);

	raw_conf = _readConf(path);
	if (raw_conf.empty()) 
    {
		std::cerr << "webserv: " << path << ": " << "Can not read default configuration file" << std::endl;
        _error = DEFAULT_CONF_ERROR;
		return false;
	}
	file.push_back("server");
	file.push_back("{");
	file.insert(file.end(), raw_conf.begin(), raw_conf.end());
	file.push_back("}");
	if (!(_parseConf(index, file, default_conf, serv))) 
    {
		std::cerr << "webserv: " << path << ": " << "Invalid default configuration file"  << std::endl;
        _error = DEFAULT_CONF_ERROR;
		return false;
	}
    return true;
}

bool ParserConf::_parseConf(size_t &index, std::vector<std::string> &file, ServerConf &parent, int server_index)
{
    std::vector<std::string>    arguments;
	std::string                 directive;
	std::map<std::string, bool (ParserConf::*)(std::vector<std::string>&, ServerConf&)>::iterator methodPtr;
	ServerConf* target = (server_index > 0) ? &_servers[server_index - 1] : &_defaultConf;
	target->setParent(&parent);

	while (index < file.size() && file[index] != "}")
	{
		if ((methodPtr = _parseMap.find(file[index])) == _parseMap.end())
		{
			if (file[index] == "location")
			{
				if (directive != "") 
				{
					if (!(this->*_parseMap[directive])(arguments, *target))
						return false;
					arguments.clear();
					directive = "";
				}
				if (!(_setLocation(*target, file, server_index, index)))
					return false;
			}
			else if (directive == "")
				return (file[index] == "}") ? true : false;
			else
				arguments.push_back(file[index]);
		}
		else
		{
			if (directive != "") 
			{
				if (!(this->*_parseMap[directive])(arguments, *target))
					return false;
				arguments.clear();
			}
			directive = methodPtr->first;
		}
		++index;
	}
	if (directive != "")
		if (!(this->*_parseMap[directive])(arguments, *target))
			return false;
	if (file[index] == "}")
		return true;
	return false;
}

bool ParserConf::_setLocation(ServerConf &parent, std::vector<std::string> &file, int &s_index, size_t &f_index, std::string name)
{
    std::vector<std::string>    arguments;
	std::string                 directive;
	std::map<std::string, bool (ParserConf::*)(std::vector<std::string>&, ServerConf&)>::iterator methodPtr;
	ServerConf	newLocation;
	ServerConf* location = &newLocation;
	ServerConf	*config = (s_index == 0) ? &_defaultConf : &_servers[s_index - 1];
	location->setParent((parent.getDefaultFlag()) ? &_defaultConf : &parent);

	if (file[++f_index] == "{" || file[f_index] == "}")
	{	
		return false;
	}
	name += (*file[f_index].begin() == '/') ? file[f_index] : ("/" + file[f_index]);
	if (s_index == 0)
	{
		_defaultConf.setLocation(name, *location);
		location = &_defaultConf.getLocation(name);
	}
	else
	{
		_servers[s_index - 1].setLocation(name, *location);
		location = &_servers[s_index - 1].getLocation(name);
	}
	if (file[++f_index] != "{")
	{
		return false;
	}
	++f_index;
	while (f_index < file.size() && file[f_index] != "}") 
	{
		if ((methodPtr = _locationParseMap.find(file[f_index])) == _locationParseMap.end()) 
		{
			if (file[f_index] == "location") 
			{
				if (directive != "") 
				{
					if(!(this->*_locationParseMap[directive])(arguments, *location))
					{
						return false;
					}
					arguments.clear();
					directive = "";
				}
				if (!(_setLocation(*location, file, s_index, f_index, name)))
					return false;
				config->setLocation(name, *location);
	//			++f_index;
	//			if (file[f_index] == "}")
	//				continue ;
			}
			else if (directive == "")
				return file[f_index] == "}" ? true : false;
			else
				arguments.push_back(file[f_index]);
		}
		else
		{
			if (directive != "") 
			{
				if(!(this->*_locationParseMap[directive])(arguments, *location))
					return false;
				arguments.clear();
				directive = "";
			}
			directive = methodPtr->first;
		}
		++f_index;
	}
	if (directive != "")
		if (!(this->*_locationParseMap[directive])(arguments, *location))
			return false;
	if (file[f_index] == "}")
		return true;
	return false;
}

std::vector<std::string> ParserConf::_readConf(const std::string &path)
{
    int							fd, bytes;
	char						buffer[CONF_READ_BUFFER + 1] = {0};
	std::string					line = "", token;
	std::vector<std::string>	result;
    size_t                      start, end = 0;

	if ((fd = open(path.c_str(), O_RDONLY)) <= 0)
    {
		std::cerr << "webserv: " << path << ": " << "Can not open configuration file" << std::endl;
        return result ;
    }
	for (bytes = CONF_READ_BUFFER; bytes > 0; bytes = read(fd, buffer, CONF_READ_BUFFER ))
    {
		buffer[bytes] = '\0';
		line += buffer;
	}
	if (bytes == -1)
    {
		std::cerr << "webserv: " << path << ": " << "Error while reading configuration file" << std::endl;
        _error = DEFAULT_CONF_ERROR;
		return result;
	}
    line += " ";
    start = line.find_first_not_of(" \n\t", 0);
	while (1) 
    {
		end = line.find_first_of(" \n\t", start);
		if (end == std::string::npos)
			break;
		token = line.substr(start, end - start);
		result.push_back(token);
        start = line.find_first_not_of(" \n\t", end);
		if (start == std::string::npos)
			break ;
	}
 	return result;
}

bool ParserConf::_setAutoIndex(std::vector<std::string> &arg, ServerConf &target)
{
	if (!arg.empty())
	{
		if (arg[0] == "on")
			target.setAutoIndex(true);
		else if (arg[0] == "off")
			target.setAutoIndex(false);
		else
		{
			std::cerr << "webserv: configuration: " << "incorrect arguments for \"autoindex\" directive" << std::endl;
			_error = READ_DIRECTIVE_ERROR;
			return false;
		}
	}
	else
	{
		std::cerr << "webserv: configuration: " << "empty argument for \"autoindex\" directive" << std::endl;
		_error = READ_DIRECTIVE_ERROR;
		return false;
	}
	return true;
}

bool ParserConf::_setListen(std::vector<std::string> &arg, ServerConf &target)
{
	size_t pos;

	if (arg.size() != 1)
	{
		std::cerr << "webserv: configuration: " << "wrong argument for \"listen\" directive" << std::endl;
		return false;
	}
	if ((pos = arg[0].find(":")) == std::string::npos) 
	{
		if (arg[0].find_first_not_of("0123456789") == std::string::npos) 
		{
			if (!(target.setListen(0, std::atoi(arg[0].c_str()))))
			{
				std::cerr << "webserv: configuration: " << "duplicate port for \"listen\" directive" << std::endl;
				return false;
			}
			return true;
		}
		std::cerr << "webserv: configuration: " << "wrong argument for \"listen\" directive" << std::endl;
		return false;
	}
	else
	{
		unsigned int host = inet_addr(arg[0].substr(0, pos).c_str());
		
		if (host == INADDR_NONE)
		{
			std::cerr << "webserv: configuration: " << "wrong host in \"listen\" directive" << std::endl;
			return false;
		}
		++pos;
		std::string checkPort = arg[0].substr(pos);

		if (checkPort.find_first_not_of("0123456789") == std::string::npos)
		{
			if (!(target.setListen(host, std::atoi(checkPort.c_str()))))
			{
				std::cerr << "webserv: configuration: " << "duplicate port for \"listen\" directive" << std::endl;
				return false;
			}
			return true;
		}
		std::cerr << "webserv: configuration: " << "wrong argument for \"listen\" directive" << std::endl;
		return false;
	}
}

bool ParserConf::_setRoot(std::vector<std::string> &arg, ServerConf &target)
{
	if (arg.size() != 1 || target.getRoot() != "")
	{
		std::cerr << "webserv: configuration: " << "wrong argument for \"root\" directive" << std::endl;
		return false;
	}
	target.setRoot(arg[0]);
	return true;
}

bool ParserConf::_setServerName(std::vector<std::string> &arg, ServerConf &target)
{
	std::vector<std::string> names;

	if (arg.size() < 1)
	{
		std::cerr << "webserv: configuration: " << "wrong argument for \"server_name\" directive" << std::endl;
		return false;
	}
	for (int i = 0; i < arg.size(); ++i)
		names.push_back(arg[i]);
	target.setServerName(names);
	return true;
}

bool ParserConf::_setErrorPage(std::vector<std::string> &arg, ServerConf &target)
{
	std::vector<int> codes;
	std::string uri = "";
	int i = -1;

	while(arg[++i].find_first_not_of("0123456789") == std::string::npos)
		codes.push_back(std::atoi(arg[i].c_str()));
	if (codes.empty() || i != arg.size() - 1 || arg[i] == "")
	{
		std::cerr << "webserv: configuration: " << "wrong argument for \"error_page\" directive" << std::endl;
		return false;
	}
	uri = arg[i];
	target.setErrorPage(codes, uri);
	return true;
}

bool ParserConf::_setClientBodyBufferSize(std::vector<std::string> &arg, ServerConf &target)
{
	if (arg.size() != 1 || arg[0].find_first_not_of("0123456789") != std::string::npos)
	{
		std::cerr << "webserv: configuration: " << "wrong argument for \"client_body_buffer_size\" directive" << std::endl;
		return false;
	}
	size_t value = std::atol(arg[0].c_str());
	target.setClientBodyBufferSize(value);
	return true;
}

bool ParserConf::_setCgiParam(std::vector<std::string> &arg, ServerConf &target)
{
	if (arg.size() != 2 || (arg[0] == "" || arg[1] == ""))
	{
		std::cerr << "webserv: configuration: " << "wrong argument for \"cgi_param\" directive" << std::endl;
		return false;
	}
	target.setCgiParam(arg[0], arg[1]);
	return true;
}

bool ParserConf::_setCgiPass(std::vector<std::string> &arg, ServerConf &target)
{
	if (arg.size() != 1 || arg[0] == "")
	{
		std::cerr << "webserv: configuration: " << "wrong argument for \"cgi_pass\" directive" << std::endl;
		return false;
	}
	target.setCgiPass(arg[0]);
	return true;
}

bool ParserConf::_setAllowedMethods(std::vector<std::string> &arg, ServerConf &target)
{
	if (arg.size() > 3 || arg.empty())
	{
		std::cerr << "webserv: configuration: " << "wrong argument for \"allow_methods\" directive" << std::endl;
		return false;	
	}
	std::vector<std::string> res;
	for (std::vector<std::string>::iterator i = arg.begin(); i != arg.end(); ++i)
	{
		if ((*i == "GET" || *i == "POST" || *i == "DELETE" || *i == "PUT") && std::find(res.begin(), res.end(), *i) == res.end())
			res.push_back(*i);
		else
		{
			std::cerr << "webserv: configuration: " << "wrong argument for \"allow_methods\" directive" << std::endl;
			return false;
		}
	}
	target.setAllowedMethods(res);
	return true;
}

bool ParserConf::_setIndex(std::vector<std::string> &arg, ServerConf &target)
{
	if (arg.empty())
	{
		std::cerr << "webserv: configuration: " << "wrong argument for \"index\" directive" << std::endl;
		return false;
	}
	target.setIndex(arg);
	return true;
}

bool ParserConf::_setAlias(std::vector<std::string> &arg, ServerConf &target)
{
	std::string empty = "";

	if (arg.size() > 1)
	{
		std::cerr << "webserv: configuration: " << "wrong argument for \"alias\" directive" << std::endl;
		return false;
	}
	else if (arg.size() == 1)
		target.setAlias(arg[0]);
	else
		target.setAlias(empty);
	return true;
}

const ServerConf &ParserConf::getDefaultServer()
{
	return _defaultConf;
}

ServerConf &ParserConf::getServer(int i)
{
	if (i < _servers_count)
	{
		return _servers[i];
	}
	else
	{
		std::cerr << "out of range" << std::endl;
		return _defaultConf;
	}
}


void ParserConf::_initMaps(void)
{
    _parseMap["listen"] = &ParserConf::_setListen;
	_parseMap["root"] = &ParserConf::_setRoot;
	_parseMap["server_name"] = &ParserConf::_setServerName;
	_parseMap["error_page"] = &ParserConf::_setErrorPage;
	_parseMap["client_body_buffer_size"] = &ParserConf::_setClientBodyBufferSize;
    _parseMap["cgi_param"] = &ParserConf::_setCgiParam;
    _parseMap["cgi_pass"] = &ParserConf::_setCgiPass;
	_parseMap["allow_methods"] = &ParserConf::_setAllowedMethods;
	_parseMap["index"] = &ParserConf::_setIndex;
    _parseMap["autoindex"] = &ParserConf::_setAutoIndex;

    _locationParseMap["root"] = &ParserConf::_setRoot;
    _locationParseMap["error_page"] = &ParserConf::_setErrorPage;
    _locationParseMap["client_body_buffer_size"] = &ParserConf::_setClientBodyBufferSize;
    _locationParseMap["cgi_param"] = &ParserConf::_setCgiParam;
	_locationParseMap["cgi_pass"] = &ParserConf::_setCgiPass;
    _locationParseMap["allow_methods"] = &ParserConf::_setAllowedMethods;
    _locationParseMap["index"] = &ParserConf::_setIndex;
    _locationParseMap["autoindex"] = &ParserConf::_setAutoIndex;
    _locationParseMap["alias"] = &ParserConf::_setAlias;
}