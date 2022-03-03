#include "CGI.hpp"

CGI::~CGI() {}

CGI::CGI(Request& request, ServerConf& conf) : _request(request), _conf(conf)
{
    			// META VARIABILI DA MANDARE AL CGI
/*			AUTH_TYPE
			CONTENT_LENGTH
			CONTENT_TYPE
			GATEWAY_INTERFACE
			PATH_INFO
			PATH_TRANSLATED
			QUERY_STRING
			REMOTE_ADDR
			REMOTE_IDENT
			REMOTE_USER
			REQUEST_METHOD
			REQUEST_URI
			SCRIPT_NAME
			SERVER_NAME
			SERVER_PORT
			SERVER_PROTOCOL
			SERVER_SOFTWARE */

            char pwd[PATH_MAX];
            std::string fullPwd;
            std::string root = _conf.getRoot();
            _env["AUTH_TYPE"] = "";
            _env["CONTENT_LENGTH"] = (_request.getBodySize() == 0) ? "0" : std::to_string(_request.getBodySize());
            if (_request.getMethod() == "GET")
                _env["CONTENT_TYPE"] = "";//"text/html";
            else
                _env["CONTENT_TYPE"] = "application/x-www-form-urlencoded";
            _env["GATEWAY_INTERFACE"] = "CGI/1.1";
            
            if (!getwd(pwd))
                throw std::runtime_error(std::string(pwd));
            else
            {
                fullPwd += pwd;
                if (root.at(0) == '.')
                    root = root.substr(1, std::string::npos);
                if (root.at(0) != '/')
                    root = "/" + root;
                if (root.back() == '/')
                    root = root.substr(0, root.size() - 1);
                fullPwd += root + _request.getPath();
                _env["PATH_INFO"] = _request.getPath();
                _env["PATH_TRANSLATED"] = _request.getPath();
            }
            _env["REDIRECT_STATUS"] = "200";
            if (_request.getMethod() == "POST")
            {
                if (!_request.getBody().empty())
                {
                _env["QUERY_STRING"] = _request.getBody();
                }
                else
                {
                _env["QUERY_STRING"] = "";
                }
            }
            else if (_request.getMethod() == "GET")
            {
                _env["QUERY_STRING"] = _request.getQuery();
            }

            _env["REMOTEaddr"] = "0";//_request.getIp();
            _env["REMOTE_IDENT"] = "";
            _env["REMOTE_USER"] = "";

            _env["REQUEST_METHOD"] = _request.getMethod();
            _env["REQUEST_URI"] = _request.getPath();//_request.getPath() + "?" + _request.getQuery();

            _env["SCRIPT_NAME"] = root + _request.getPath();//_env["PATH_INFO"];
            _env["SCRIPT_FILENAME"] = root + _request.getPath();
            //Temporary
            if (_conf.getServerName().empty())
                _env["SERVER_NAME"] = "localhost";
            else
                _env["SERVER_NAME"] = _conf.getServerName().at(0);
            
            _env["SERVER_PORT"] = std::to_string(_conf.getPort());
            _env["SERVER_PROTOCOL"] = "HTTP/1.1";
            _env["SERVER_SOFTWARE"] = "ft_webserv";
      //      _env.insert(_conf.getCgiParam().begin(), _conf.getCgiParam().end());
            //maybe need to implement for test
            //headers_cgi["HTTP_X_SECRET_HEADER_FOR_TEST"] = "";
}

std::string &CGI::getResponse()
{
    return _response;
}

void CGI::execute()
{
    size_t envSize = _env.size();
    char **env = (char**)calloc(envSize + 1, sizeof(char*));
    if (!env)
        throw std::runtime_error(strerror(errno));
    int i = 0;
    for (EnvMap::iterator it = _env.begin(); it != _env.end(); ++it, ++i)
    { 
    //    std::cout << it->first << "=" << it->second << "\n";
        std::string field = it->first + "=" + it->second; 
        env[i] = strdup(field.c_str());
        std::cout << i << ":" << env[i] << std::endl;
    }
    env[envSize] = NULL;

    char **argv = (char**)calloc(3, sizeof(char*));
    if (!argv)
        throw std::runtime_error(strerror(errno));

    std::string cgiPath = "/Users/lcarry/Desktop/webserv_git/" + _conf.getCgiPass();
    argv[0] = strdup(cgiPath.c_str()); //???? <path>/<cgi> is needed
    EnvMap::iterator it = _env.find("PATH_INFO");//std::find(_env.begin(), _env.end(), "PATH_INFO");
    argv[1] = strdup("/Users/lcarry/Desktop/webserv_git/YoupiBanane/directory/youpi.bla");
    argv[2] = NULL;

    int fd[2];
    pid_t pid;

    char buff[BUFSIZ + 1];

    int fileIn = open("input.tmp", O_RDWR | O_CREAT | O_TRUNC, 0666);
    int fileOut = open("output.tmp", O_RDWR | O_CREAT | O_TRUNC, 0666);
    if (write(fileIn, _request.getBody().c_str(), _request.getBodySize()) < 0)
        throw std::runtime_error("Can not send body to cgi"); 
    lseek(fileIn, 0, SEEK_SET);

    // for (int j = 0; j < 2; j++)
    // {
    //     write(1, argv[j], strlen(argv[j]));
    //     std::cout << std::endl;
    // }

    if (fileIn < 0 || fileOut < 0 || pipe(fd) < 0 || (pid = fork()) < 0)
    {
        std::string errMsg = "Can not to run the cgi: " + std::string(strerror(errno));
        // _response = "Status: 500\r\n";
        throw std::runtime_error(errMsg.c_str());
    }

    else if (pid == 0)
    {
        dup2(fileIn, 0);
        close(fileIn);
        dup2(fileOut, 1);
        close(fileOut);
        
        int ret = execve(argv[0], NULL, env);
        if (ret < 0)
        {
            char s[20] = "Status: 500\r\n\r\n\0";
            std::cerr << "Execve error\n";
            write(1, s, strlen(s));
        }
        exit(ret);
    }
    else
    {
     /*   size_t sentBytes = write(fd[1], _request.getBody().c_str(), _request.getBodySize());
        if (sentBytes < 0)
        {
            // _response = "Status: 500\r\n";
            throw std::runtime_error("Can not send body to cgi"); //надо установить нужный код ошибки в реквест
        }*/
        close(fd[0]);
        close(fd[1]);
      
        int status;
        waitpid(pid, &status, 0);
        
        lseek(fileOut, 0, 0);
        size_t readedBytes = read(fileOut, buff, BUFSIZ);
        if (readedBytes < 0)
        {
            // _response = "Status: 500\r\n";
            throw std::runtime_error("Can not read cgi response"); //надо установить нужный код ошибки в реквест
        }
        close(fileOut);
        close(fileIn);
        free(argv);
        free(env);
        buff[readedBytes] = '\0';
        std::cout << buff << "\n";
        _response = buff;
    }
}
