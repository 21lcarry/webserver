#include "Response.hpp"
#include "CGI.hpp"

Response::Response(const Request &request, ServerConf &config) : _request(request), _config(config), _code(request.getCode())
{
    std::cout << "++\n";
    std::cout << request.getRaw();
    std::cout << "++\n"; 
    _initCodeMap();
    _initMethodMap();
    _initHeaders();
    if (_code == 400)
        _requestError();
    else if (request.getBodySize() > config.getClientBodyBufferSize())
    {
        std::cout << "413 code: size: " << request.getBodySize() << " max_size: " << config.getClientBodyBufferSize() << std::endl;
        _code = 413;
        _requestError();
    }
    else if (std::find(config.getAllowedMethods().begin(), config.getAllowedMethods().end(), request.getMethod()) == config.getAllowedMethods().end())
    {
        _code = 405;
        _requestError();
    }
    else if (_code == 501)
        _requestError();
    else
        (this->*_methodMap[request.getMethod()])();
}

bool Response::_checkPath(const std::string& path)
{
	struct stat st;
    std::string temp;

    temp = _config.getRoot();
    if (temp.size() > 1 && temp[0] != '.')
        if (*(temp.end() - 1) == '/')
            temp = "/" + temp.substr(0, temp.size() - 1);
    std::cout << temp << ":" << path <<"!!!!\n";
    if (temp.size() > 0 && (temp[0] != '.' && temp[0] == '/'))
        _full_path = "." + temp + path;
    else if (temp.size() < 1 && (path[0] != '.' && path[0] == '/'))
        _full_path = "." + path;
    else
        _full_path = temp + path;
	if (stat(_full_path.c_str(), &st) == 0 )
	{
		if (st.st_mode & S_IFREG)
        {
            char			buffer[100];
            struct tm		*tm;

            tm = gmtime(&st.st_mtime);
            strftime(buffer, 100, "%a, %d %b %Y %H:%M:%S GMT", tm);
            _headers["Last-Modified"] = std::string(buffer);
			return true;
        }
		else
			return false;
	}
	else
		return false;
}

std::string Response::_getAutoIndexPage()
{
    in_addr addr;
    addr.s_addr = _config.getListen()[0].ip;
    std::string dirName = _request.getPath(), ip = inet_ntoa(addr);
    DIR *dir = opendir(_full_path.c_str());

    std::string page =\
    "<!DOCTYPE html>\n\
    <html>\n\
    <head>\n\
            <title>" + dirName + "</title>\n\
    </head>\n\
    <body>\n\
    <h1>INDEX</h1>\n\
    <p>\n";

    _code = 200;
    if (dir == NULL) {
        _code = 403;//500?
        _requestError();
        return "";
    }
    if (dirName[0] != '/')
        dirName = "/" + dirName;
    if (dirName.size() > 1 && *(dirName.end() - 1) == '/')
        dirName = dirName.substr(0, dirName.size() - 1);
    for (struct dirent *dirEntry = readdir(dir); dirEntry; dirEntry = readdir(dir)) {
        if (dirEntry->d_name[0] != '/' && dirName.size() > 1)
            page += "\t\t<p><a href=\"http://" + ip + ":" + std::to_string(_config.getListen()[0].port)\
        + dirName + "/" + dirEntry->d_name + "\">" + dirEntry->d_name + "</a></p>\n";
        else
            page += "\t\t<p><a href=\"http://" + ip + ":" + std::to_string(_config.getListen()[0].port)\
        + dirName + dirEntry->d_name + "\">" + dirEntry->d_name + "</a></p>\n";
    }
    page +="\
    </p>\n\
    </body>\n\
    </html>\n";
    closedir(dir);
    return page;
}


void Response::_methodPOST()
{
    std::string page = "";

    if (_config.getCgiPass() != "")
    {
        _cgiHandler(page);
        if (_code == 500)
            _requestError();
        else if (_code == 200)
            _writeHeader(page);
    }
    else
    {
        _code = 204;
        _writeHeader(page);
    }
}

void Response::_methodPUT()
{
	std::ofstream	file;
    std::string page = "";

    if (_checkPath(_request.getPath()))
    {
		file.open(_full_path.c_str());
        if (file.is_open() == false)
        {
            _code = 403;
            _requestError();
            return ;
        }
		file << _request.getBody();
		file.close();
        _code = 204;
        _writeHeader(page);
    }
	else
	{
		file.open(_full_path.c_str(), std::ofstream::out | std::ofstream::trunc);
		if (file.is_open() == false)
        {
            _code = 403;
            _requestError();
            return ;
        }
		file << _request.getBody();
		file.close();
        _code = 201;
        _writeHeader(page);
	}
}

void Response::_methodDELETE()
{
    std::string page = "";
	if (_checkPath(_request.getPath()))
	{
		if (remove(_full_path.c_str()) == 0)
        {
			_code = 204;
            _writeHeader(page);
        }
		else
			_code = 403;
	}
	else
		_code = 404;
	if (_code == 403 || _code == 404)
		_requestError();
}

void Response::_cgiHandler(std::string &content)
{
    try
    {
        CGI	cgi(_request, _config);
        cgi.execute();
        content = cgi.getResponse();
        /* code */
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
        _code = 500;
        return ;
    }
      std::cout << "___\n" << content << "___\n";  
    size_t		i = 0;
    size_t		j = content.size() - 2;
	while (content.find("\r\n\r\n", i) != std::string::npos || content.find("\r\n", i) == i)
	{
		std::string	str = content.substr(i, content.find("\r\n", i) - i);
		if (str.find("Status: ") == 0)
			_code = std::atoi(str.substr(8, 3).c_str());
		else if (str.find("Content-type: ") == 0)
			_headers["Content-Type: "] = str.substr(14, str.size());
		i += str.size() + 2;
	}
	while (content.find("\r\n", j) == j)
		j -= 2;

	content = content.substr(i, j - i);


}

void Response::_methodGET()
{
    std::string html;
    struct stat st;

	if (_config.getCgiPass() != "")
	{
        _cgiHandler(html);
        if (_code == 500)
            _requestError();
        else if (_code == 200)
            _writeHeader(html);
	}
    else if (_checkPath(_request.getPath()))
    {
        html = _readHtml(_full_path);
        if (_code == 403)
            _requestError();
        else
        {
            _code = 200;
            _writeHeader(html);
        }
    }
    else if (_config.getIndex().size() != 0 && (stat(_full_path.c_str(), &st) == 0 && (st.st_mode & S_IFDIR)))
    {
        std::string path = ((*_full_path.rbegin() == '/') ? _full_path : _full_path + "/")  + _config.getIndex()[0]; //первый в директиве index
        bool file = (stat(path.c_str(), &st) == 0 && (st.st_mode & S_IFREG));

        html = _readHtml(path);
        if (_code == 403 && file)
            _requestError();
        else if (!file)
        {
            _code = 404;
            _requestError();
        }
        else
        {
            _code = 200;
            _headers["Content-Type"] = "text/html";
            _writeHeader(html);
        }
    }
    else if (_config.getAutoIndex() && (stat(_full_path.c_str(), &st) == 0 && (st.st_mode & S_IFDIR)))
    {
        html = _getAutoIndexPage();
        if (_code == 200)
        {
             _headers["Content-Type"] = "text/html";
            _writeHeader(html);
        }
        else
            _requestError();
    }
    else
    {
        _code = 404;
        _requestError();
    }
}

void Response::_initMethodMap()
{
    _methodMap["GET"] = &Response::_methodGET;
	_methodMap["POST"] = &Response::_methodPOST;
	_methodMap["PUT"] = &Response::_methodPUT;
	_methodMap["DELETE"] = &Response::_methodDELETE;
}

void Response::_initCodeMap()
{
    _response_code[100] = "Continue";
	_response_code[200] = "OK";
	_response_code[201] = "Created";
	_response_code[204] = "No Content";
	_response_code[400] = "Bad Request";
	_response_code[403] = "Forbidden";
	_response_code[404] = "Not Found";
	_response_code[405] = "Method Not Allowed";
	_response_code[413] = "Payload Too Large";
	_response_code[500] = "Internal Server Error";
    _response_code[501] = "Not Implemented";
}

std::string Response::_getContentType()
{
	std::string type = _request.getPath().substr(_request.getPath().rfind(".") + 1,\
                       _request.getPath().size() - _request.getPath().rfind("."));

	if (type == "html")
		return ("text/html");
	else if (type == "css")
		return ("text/css");
	else if (type == "js")
		return ("text/javascript");
	else if (type == "jpeg" || type == "jpg")
		return ("image/jpeg");
	else if (type == "png")
		return ("image/png");
	else if (type == "bmp")
		return ("image/bmp");
	else
		return ("text/plain");
}

void Response::_initHeaders()
{
    if (_config.getAllowedMethods().size() > 0)
    {
        for(std::vector<std::string>::iterator it = _config.getAllowedMethods().begin(); it != (_config.getAllowedMethods().end() - 1); ++it)
            _headers["Allow"] = *it + " ";
        _headers["Allow"] = *(_config.getAllowedMethods().end() - 1);
    }
    _headers["Content-Language"] = "en-US";
	_headers["Content-Length"] = "0";
	_headers["Content-Location"] = _request.getPath();
	_headers["Content-Type"] = "text/plain";
    {
        char			buffer[100];
        struct timeval	tv;
        struct tm		*tm;

        gettimeofday(&tv, NULL);
        tm = gmtime(&tv.tv_sec);
        strftime(buffer, 100, "%a, %d %b %Y %H:%M:%S GMT", tm);
        _headers["Date"] = std::string(buffer);
    }
    _headers["Last-Modified"] = "";
	_headers["Location"] = "";
	_headers["Retry-After"] = "";
	_headers["Server"] = WEBSERV;
	_headers["Transfer-Encoding"] = "identity";
	_headers["Www-Authenticate"] = "";
}

std::string Response::_readHtml(std::string &path)
{
	std::ofstream		file;
	std::stringstream	buffer;
    std::string err = "Cannot open error page, path: " + path + "\n";

	file.open(path.c_str(), std::ifstream::in);
	if (file.is_open() == false)
    {
        _code = 403;
		return (err);
    }
    buffer << file.rdbuf();
	file.close();
	_headers["Content-Type"] = (_code == 200) ? _getContentType() : "text/html";
    std:: cout << buffer.str() << std::endl;
    _headers["Content-Length"] = (buffer.str().size() > 0) ? std::to_string(buffer.str().size()) : "0";
	return (buffer.str());
}

void Response::_writeHeader(std::string &page)
{
    _header = "HTTP/1.1 " + std::to_string(_code) + " " + _response_code[_code] + "\r\n";
    for (std::map<std::string, std::string>::iterator it = _headers.begin(); it != _headers.end(); ++it)
        if (it->first != "Allow" && it->second != "")
            _header += it->first + ": " + it->second + "\r\n";
    if (page.size() > 0 && _request.getMethod() == "GET")
        _header += "\r\n" + page;
}

void Response::_requestError()
{
    std::string html_page;

    if (_request.getMethod() == "GET")
        html_page = _readHtml(_config.getErrorPage()[_code]);

    _header = "HTTP/1.1 " + std::to_string(_code) + " " + _response_code[_code] + "\r\n";
    for (std::map<std::string, std::string>::iterator it = _headers.begin(); it != _headers.end(); ++it)
    {
        if (it->first != "Allow" && it->second != "")
            _header += it->first + ": " + it->second + "\r\n";
        else if (_code == 405 && it->first == "Allow")
            _header += it->first + ": " + it->second + "\r\n";
    }
    if (_request.getMethod() == "GET")
        _header += "\r\n" + html_page;
}

const std::string &Response::getHeader() const
{
    return _header;
}