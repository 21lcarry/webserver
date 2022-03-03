#ifndef RESPONSE_HPP
#define RESPONSE_HPP
#include "webserv.h"
#include "Request.hpp"
#include "ServerConf.hpp"

class Response
{
    public:
        Response(const Request &request, ServerConf &config);
        ~Response() {}

        const std::string &getHeader() const;
    private:
        Response();
        Response(Response &o);
        Response &operator=(Response &o);

        void _initCodeMap();
        void _initHeaders();
        void _initMethodMap();
        /*Response func*/
        void _requestError();
        /****Methods****/
        void _methodGET();
        void _methodPOST();
        void _methodPUT();
        void _methodDELETE();
        /***************/
        std::string _readHtml(std::string &path);
        void _writeHeader(std::string &page);
        bool _checkPath(const std::string &path);
        std::string _getAutoIndexPage();
        std::string _getContentType();
        void _cgiHandler(std::string &content);

        ServerConf _config;
        Request     _request;
        std::map<int, std::string> _response_code;
        std::map<std::string, std::string> _headers;
        std::map<std::string, void (Response::*)()> _methodMap;
        std::string _header;
        std::string _full_path;
        int _code;
};

#endif 