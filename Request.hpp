#ifndef REQUEST_HPP
#define REQUEST_HPP
#include "webserv.h"

class Request
{
private:
    std::string _rawRequest;
    std::string _method;
    std::string _path;
    std::string _version;
    std::string _body;
    std::string _query;
    std::string _ip;
    size_t      _body_size;
    std::map<std::string, std::string> _headers;
    int         _code;

    std::string _collectChunk(const std::string &body);
    void parsing();
    void firstLine(const std::string &str);
    void checkMethod();
    std::string getLine(const std::string &str, size_t &i);
    std::string getKey(const std::string &str);
    std::string getValue(const std::string &str);
    void setQuery();
    void trim(std::string &str, char c);

public:
    Request(std::string &str, std::string &ip);
    Request() {}
    ~Request() {}
    Request(const Request&);
    Request &operator=(const Request&);

    const std::string &getMethod() const;
    const int &getCode() const;
    const std::string &getRaw() const;
    int getBodySize() const;
    const std::string &getPath() const;
    const std::string &getBody() const;
    const std::string &getQuery() const;
    const std::string &getIp() const;
};

#endif