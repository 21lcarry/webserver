#pragma once
#include "webserv.h"
#include "Request.hpp"
#include "ServerConf.hpp"

typedef std::map<std::string, std::string> EnvMap;

class CGI
{
private:
    Request& _request;
    std::string _response;
    EnvMap _env;
    ServerConf& _conf;
    char** getEnv();
public:
    CGI(Request &request, ServerConf& conf);
    ~CGI();

    std::string &getResponse();
    void execute();
};
