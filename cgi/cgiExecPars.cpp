#include "cgiHeader.hpp"
#include "../AllServer/HttpServer.hpp"

bool Cgi::parseOutput()
{
    std::ifstream file(outFile.c_str());
    std::string line;
    bool readHeader = true;
    cgiStatusCode = 200;
    cgiHeader = "";
    cgiBody = "";

    while (std::getline(file, line))
    {
        if (!line.empty() && line.back() == '\r')
            line.pop_back();
        if (readHeader)
        {
            if (line.empty())
            {
                readHeader = false;
                continue;
            }
            size_t colonPos = line.find(':');
            if (colonPos == std::string::npos || colonPos == 0)
                return false;
            std::string headerName = line.substr(0, colonPos);
            std::string headerValue = line.substr(colonPos + 1);
            headerValue.erase(0, headerValue.find_first_not_of(" \t"));
            if (headerValue.empty())
                return false;
            if (headerName == "Status")
                cgiStatusCode = std::atoi(headerValue.c_str());
            else if (headerName == "Content-Length")
                cgiContentLength = std::atoi(headerValue.c_str());
            else if (headerName == "Content-Type")
                cgiHeader += headerName + ": " + headerValue + "\r\n";
            else
                cgiHeader += headerName + ": " + headerValue + "\r\n";
        }
        else
        {
            if (line.find("\r") != std::string::npos)
                return false;
            if (line.empty())
                continue;
            cgiBody += line + "\n";
        }
    }
    cgiFileSize = cgiBody.size();
    return true;
}


char    **Cgi::cgiEnvVariables(Request &req, std::string _pathInfo)
{
    char **envp = new char*[15];

    envp[0] = new char[strlen("REQUEST_METHOD=") + std::strlen(req.GetHeaderValue("method").c_str()) + 1];
    std::strcpy(envp[0], "REQUEST_METHOD=");
    std::strcat(envp[0], req.GetHeaderValue("method").c_str());
    envp[1] = new char[std::strlen("SCRIPT_NAME=") + std::strlen(fullPath.c_str()) + 1];
    std::strcpy(envp[1], "SCRIPT_NAME=");
    std::strcat(envp[1], fullPath.c_str());
    envp[2] = new char[std::strlen("SCRIPT_FILENAME=") + std::strlen(scriptFile.c_str()) + 1];
    std::strcpy(envp[2], "SCRIPT_FILENAME=");
    std::strcat(envp[2], scriptFile.c_str());
    envp[3] = new char[std::strlen("QUERY_STRING=") + std::strlen(req.GetHeaderValue("query").c_str()) + 1];
    std::strcpy(envp[3], "QUERY_STRING=");
    std::strcat(envp[3], req.GetHeaderValue("query").c_str());
    envp[4] = new char[std::strlen("SERVER_PROTOCOL=") + std::strlen("HTTP/1.1") + 1];
    std::strcpy(envp[4], "SERVER_PROTOCOL=");
    std::strcat(envp[4], "HTTP/1.1");
    envp[5] = new char[std::strlen("GATEWAY_INTERFACE=") + std::strlen("CGI/1.1") + 1];
    std::strcpy(envp[5], "GATEWAY_INTERFACE=");
    std::strcat(envp[5], "CGI/1.1");
    envp[6] = new char[std::strlen("SERVER_SOFTWARE=") + std::strlen("webSERV/1.0") + 1];
    std::strcpy(envp[6], "SERVER_SOFTWARE=");
    std::strcat(envp[6], "webSERV/1.0");
    envp[7] = new char[std::strlen("PATH_INFO=") + std::strlen(_pathInfo.c_str()) + 1];
    std::strcpy(envp[7], "PATH_INFO=");
    std::strcat(envp[7], _pathInfo.c_str());
    envp[8] = new char[std::strlen("CONTENT_TYPE=") + std::strlen(req.GetHeaderValue("content-type").c_str()) + 1];
    std::strcpy(envp[8], "CONTENT_TYPE=");
    std::strcat(envp[8], req.GetHeaderValue("content-type").c_str());
    envp[9] = new char[std::strlen("CONTENT_LENGTH=") + std::strlen(req.GetHeaderValue("content-length").c_str()) + 1];
    std::strcpy(envp[9], "CONTENT_LENGTH=");
    std::strcat(envp[9], req.GetHeaderValue("content-length").c_str());
    envp[10] = new char[std::strlen("SERVER_NAME=") + std::strlen(req.GetHeaderValue("host").c_str()) + 1];
    std::strcpy(envp[10], "SERVER_NAME=");
    std::strcat(envp[10], req.GetHeaderValue("host").c_str());
    envp[11] = new char[std::strlen("SERVER_PORT=") + std::strlen(req.GetServerDetails().ServerPort.c_str()) + 1];
    std::strcpy(envp[11], "SERVER_PORT=");
    std::strcat(envp[11], req.GetServerDetails().ServerPort.c_str());
    envp[12] = new char[std::strlen("HTTP_COOKIE=") + std::strlen(req.GetHeaderValue("cookie").c_str()) + 1];
    std::strcpy(envp[12], "HTTP_COOKIE=");
    std::strcat(envp[12], req.GetHeaderValue("cookie").c_str());
    envp[13] = new char[std::strlen("REDIRECT_STATUS=") + 4];
    std::strcpy(envp[13], "REDIRECT_STATUS=");
    std::strcat(envp[13], "200");
    envp[14] = NULL;
    return envp;
}

void    Cgi::execCgi(char **envp)
{
    const char* argv[5];
    if (std::strstr(scriptFile.c_str(), ".py"))
    {
        argv[0] = (char*)"python3";
        argv[1] = scriptFile.c_str();
        argv[2] = NULL;
        execve("/usr/bin/python3", (char *const *)argv, envp);
    }
    else if (std::strstr(scriptFile.c_str(), ".cgi"))
    {
        argv[0] = scriptFile.c_str();
        argv[1] = NULL;
        execve(scriptFile.c_str(), (char *const *)argv, envp);
    }
    else if (std::strstr(scriptFile.c_str(), ".php"))
    {
        argv[0] = (char*)"php";
        argv[1] = (char*)"-d";
        argv[2] = (char*)"display_errors=0";
        argv[3] = scriptFile.c_str();
        argv[4] = NULL;
        execve("/usr/bin/php", (char *const *)argv, envp);
    }
    if (envp)
    {
        for (int i = 0; envp[i]; i++)
            delete[] envp[i];
        delete[] envp;
    }
    std::exit(1);
}


void Cgi::executeCgiScript(Request &req)
{
    outFile = "/tmp/cgiOutput_" + uniq;
    inpFile = req.GetCgiFileName();
    pid = fork();
    if (pid == -1)
    {
        responseErrorcgi(500, " Internal Server Error", req);
        return ;
    }
    else if (pid == 0)
    {
        int inFd = open(inpFile.c_str(), O_RDONLY);
        if (inFd != -1)
        { 
            dup2(inFd, STDIN_FILENO);
            close(inFd);
        }
        int outFd = open(outFile.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (outFd != -1)
        {
            dup2(outFd, STDOUT_FILENO);
            dup2(outFd, STDERR_FILENO);
            close(outFd);
        }
        std::string scriptDir = scriptFile.substr(0, scriptFile.find_last_of("/"));
        if (!scriptDir.empty())
        {
            if (chdir(scriptDir.c_str()) == -1)
                std::exit(1);
        }
        fullPath = scriptFile;
        scriptFile = scriptFile.substr(scriptFile.find_last_of("/") + 1);
        envp = cgiEnvVariables(req, pathInfo);
        execCgi(envp);
    }
    else
    {
        startTime = time(NULL);
        cgistatus = CGI_RUNNING;
        if (req.ctx)
        {
            req.ctx->is_cgi = true;
            req.ctx->cgi_pid = pid;
        }
        struct kevent kev;
        EV_SET(&kev, pid, EVFILT_PROC, EV_ADD | EV_ENABLE, NOTE_EXIT, 0, req.ctx);
        kevent(kq, &kev, 1, NULL, 0, NULL);
        if (req.ctx)
        {
            req.ctx->registered_procs.push_back(pid);
            req.ctx->cgi_pid = pid;
        }
        EV_SET(&kev, pid, EVFILT_TIMER, EV_ADD | EV_ENABLE, NOTE_SECONDS, 10, req.ctx);
        if (kevent(kq, &kev, 1, NULL, 0, NULL) == -1)
            std::cout << "\033[31mkevent failed for pid " << pid << ": " << strerror(errno) << " EVFILT_TIMER CGI\033[0m" << std::endl;
    }
}