#ifndef PARSERCONF_HPP
# define PARSERCONF_HPP
# include "webserv.h"
# include "ServerConf.hpp"
# define IS_LOCATION_CONF 1

class ParserConf
{
    public:
        ParserConf(const std::string &path);
        ~ParserConf();

        const ServerConf &getDefaultServer();
        ServerConf &getServer(int i);
        std::vector<ServerConf> getConfigs();
       // std::vector<ServerConf> _servers;
    private:
        ParserConf(const ParserConf &o);
        ParserConf &operator=(const ParserConf &o);
        int _error;

        /* основные функции                  */
        void _initMaps();
        bool _setDefaultConf(const std::string &path);
        std::vector<std::string> _readConf(const std::string &path);
        bool _parseConf(size_t &index, std::vector<std::string> &file, ServerConf &parent, int server_index);
        bool _initConfigArray(std::vector<std::string> &file);
        bool _parseConfFile(std::vector<std::string> &file);
        /*****************************/

        /* формирование конфигураций         */
        bool _setLocation(ServerConf &parent, std::vector<std::string> &file, int &s_index, size_t &f_index, std::string name = "");
        /*************************************/

        /* обьекты конфигураций      */
        ServerConf              _defaultConf;
        ServerConf*             _servers;
        int                 _servers_count;
        /*************************************/

        /* обработка директив                */
        bool _setAutoIndex(std::vector<std::string> &arg, ServerConf &target);
        bool _setListen(std::vector<std::string> &arg, ServerConf &target);
        bool _setRoot(std::vector<std::string> &arg, ServerConf &target);
        bool _setServerName(std::vector<std::string> &arg, ServerConf &target);
        bool _setErrorPage(std::vector<std::string> &arg, ServerConf &target);
        bool _setClientBodyBufferSize(std::vector<std::string> &arg, ServerConf &target);
        bool _setCgiParam(std::vector<std::string> &arg, ServerConf &target);
        bool _setCgiPass(std::vector<std::string> &arg, ServerConf &target);
        bool _setAllowedMethods(std::vector<std::string> &arg, ServerConf &target);
        bool _setIndex(std::vector<std::string> &arg, ServerConf &target);
        bool _setAlias(std::vector<std::string> &arg, ServerConf &target);
        /*************************************/
        void _setDefault(ServerConf &target, int flag = 0);

        /* указатели на обработчики директив */
        std::map<std::string, bool (ParserConf::*)(std::vector<std::string>&, ServerConf&)>   _locationParseMap;
        std::map<std::string, bool (ParserConf::*)(std::vector<std::string>&, ServerConf&)>   _parseMap;
        /*************************************/
};

#endif