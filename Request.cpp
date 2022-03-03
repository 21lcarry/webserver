#include "Request.hpp"

Request::Request(std::string &str, std::string& ip) : _rawRequest(str), _method(""), _path(""), _version(""), _body(""), _code(200), _ip(ip)
{

	this->_headers["Authorization"] = "";
	this->_headers["Accept-Charsets"] = "";
	this->_headers["Accept-Language"] = "";
	this->_headers["Allow"] = "";
	this->_headers["Content-Language"] = "";
	this->_headers["Content-Length"] = "";
	this->_headers["Content-Location"] = "";
	this->_headers["Content-Type"] = "";
	this->_headers["Date"] = "";
	this->_headers["Host"] = "";
	this->_headers["Last-Modified"] = "";
	this->_headers["Location"] = "";
	this->_headers["Referer"] = "";
	this->_headers["Retry-After"] = "";
	this->_headers["Server"] = "";
	this->_headers["Transfer-Encoding"] = "";
	this->_headers["User-Agent"] = "";
	this->_headers["Www-Authenticate"] = "";
	this->_headers["Connection"] = "Keep-Alive";
	this->parsing();
}

Request &Request::operator=(const Request &o)
{
	if (this != &o)
	{
		_rawRequest = o._rawRequest;
		_method = o._method;
		_path = o._path;
		_version = o._version;
		_body = o._body;
		_body_size = o._body_size;
		_code = o._code;
		_headers = o._headers;
	}
	return *this;
}

Request::Request(const Request &o)
{
	if (this != &o)
		*this = o;
}

void Request::checkMethod()
{
	if (_method != "GET" && _method != "POST" && _method != "DELETE" && _method != "PUT")
		_code = 501;
}

void Request::firstLine(const std::string &str)
{
	size_t	i, j;
	std::string	line;

	i = str.find_first_of('\n');
	line = str.substr(0, i);
	i = line.find_first_of(' ');

	if (i == std::string::npos)
	{
		this->_code = 400;
		return ;
	}
	this->_method.assign(line, 0, i);
	if ((j = line.find_first_not_of(' ', i)) == std::string::npos)
	{
		this->_code = 400;
		return ;
	}
	if ((i = line.find_first_of(' ', j)) == std::string::npos)
	{
		this->_code = 400;
		return ;
	}
	this->_path.assign(line, j, i - j);
		if ((i = line.find_first_not_of(' ', i)) == std::string::npos)
	{
		this->_code = 400;
		return ;
	}
	if (line[i] == 'H' && line[i + 1] == 'T' && line[i + 2] == 'T' &&
			line[i + 3] == 'P' && line[i + 4] == '/')
	this->_version.assign(line, i + 5, 3);
	if (this->_version != "1.0" && this->_version != "1.1")
	{
		this->_code = 400;
		return ;
	}
	this->checkMethod();
}

std::string Request::getLine(const std::string &str, size_t& i)
{
	std::string		res;
	size_t			j = 0;

	if (i == std::string::npos)
		return "";
	j = str.find_first_of('\n', i);
	res = str.substr(i, j - i);
	if (res[res.size() - 1] == '\r')
		if (res.size())
			res.resize(res.size() - 1);
	i = (j == std::string::npos) ? j : (j + 1);
	return res;
}

void Request::trim(std::string &str, char c)
{
	size_t	i;

	if (!str.size())
		return ;
	i = str.size();
	while (i && str[i - 1] == c)
		--i;
	str.resize(i);
	for (i = 0; str[i] == c; ++i)
		;
	str = str.substr(i, std::string::npos);
}

std::string Request::getKey(const std::string &str)
{
	std::string res;
	int i = -1;

	while (str[++i] && str[i] != ':')
		res += str[i];
	trim(res, ' ');
	std::transform(res.begin(), res.end(),res.begin(), ::tolower);
	i = 0;
	res[i] = std::toupper(res[i]);
	while((i = res.find_first_of('-', i + 1)) != std::string::npos)
	{
		if (i + 1 < str.size())
			res[i + 1] = std::toupper(res[i + 1]);
	}
	return res;
}

std::string Request::getValue(const std::string &str)
{
	size_t i;
	std::string	res;

	i = str.find_first_of(':');
	i = str.find_first_not_of(' ', i + 1);
	if (i != std::string::npos)
		res.append(str, i, std::string::npos);
	trim(res, ' ');
	return (res);
}

const std::string &Request::getRaw() const
{
	return _rawRequest;
}

std::string Request::_collectChunk(const std::string &body)
{
	std::string	chunks = body.substr(body.find("\r\n\r\n") + 4, body.size() - 1);
	std::string	subchunk = chunks.substr(0, 100);
	std::string	result = "";
	int		chunksize = strtol(subchunk.c_str(), NULL, 16);
	size_t		i = 0;
	std::cout << "---------------\n";
std::cout << chunksize << "\n\n";
	while (chunksize)
	{
		i = chunks.find("\r\n", i) + 2;
		result += chunks.substr(i, chunksize);
		i = chunks.find("\r\n", i) + 2;
		subchunk = chunks.substr(i, 100);
		chunksize = strtol(subchunk.c_str(), NULL, 16);
		std::cout << subchunk.c_str() << std::endl;
	}

	return (result + "\r\n\r\n");
}

void Request::parsing()
{
	std::string key, value, line;
	size_t i = 0;

	this->firstLine(getLine(_rawRequest, i));
	while ((line = getLine(_rawRequest, i)) != "\r" && line != "" && this->_code != 400)
	{
		key = getKey(line);
		value = getValue(line);
		if (this->_headers.count(key))
				this->_headers[key] = value;
	}
	while (_rawRequest[i] == '\n' || _rawRequest[i] == '\r')
		++i;
	if (i != std::string::npos)
	{
		if (_headers["Transfer-Encoding"] == "chunked")
			_body = _collectChunk(_rawRequest);
		else
			_body = _rawRequest.substr(i, std::string::npos);
	}
	_body_size = _body.size();
	setQuery();
/*
	std::cout << "Method: " << _method << '\n';
	std::cout << "Path: " << _path << '\n';
	std::cout << "Version: " << _version << '\n';
	std::cout << "Code: " << _code << '\n';
	std::cout << "Headers:\n";
	for (std::map<std::string, std::string>::iterator it = _headers.begin(); it != _headers.end(); ++it)
		if (it->second != "")
			std::cout << it->first << " : " << it->second << '\n'; */
}

void Request::setQuery()
{
	size_t		i;

	i = _path.find_first_of('?');
	if (i != std::string::npos)
	{
		_query.substr(i + 1, std::string::npos);
		_path = _path.substr(0, i);
	}
}

const int &Request::getCode() const
{
	return _code;
}

int Request::getBodySize() const
{
	return _body_size;
}

const std::string &Request::getMethod() const
{
	return _method;
}

const std::string &Request::getPath() const
{
	return _path;
}

const std::string &Request::getBody() const
{
	return _body;
}

const std::string &Request::getQuery() const
{
	return _query;
}

const std::string &Request::getIp() const
{
	return _ip;
}