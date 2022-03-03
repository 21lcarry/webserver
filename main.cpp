#include "Webserver.hpp"
#include "ParserConf.hpp"

int main(int ac, char **av)
{
    if (ac == 2)
    {
        std::vector<ServerConf> configs;
        Webserver webserv;
        ParserConf servers(av[1]);
        configs = servers.getConfigs();
        
        for (std::vector<ServerConf>::iterator it = configs.begin(); it != configs.end(); ++it)
        {
            Server serv(*it);
            webserv.setServer(serv);
        }
        webserv.run();
    }
    else
    {
        std::cerr << "webserv: Error invalid numbers of argument" << std::endl;
        exit(EXIT_FAILURE);
    }
    return 0;
}