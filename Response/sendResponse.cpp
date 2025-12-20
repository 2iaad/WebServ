#include "../AllServer/HttpServer.hpp"
#include "responseHeader.hpp"

bool Response::sendHeaders(size_t chunkSize)
{
    if (headerSent < headers.size())
    {
        size_t left = headers.size() - headerSent;
        size_t sendNow = std::min(chunkSize, left);
        chunk = headers.substr(headerSent, sendNow);
        headerSent += sendNow;
        return true;
    }
    if (_cgi.getCgiHeaderSent() < _cgi.getCgiHeader().size())
    {
        size_t left = _cgi.getCgiHeader().size() - _cgi.getCgiHeaderSent();
        size_t sendNow = std::min(chunkSize, left);
        chunk = _cgi.getCgiHeader().substr(_cgi.getCgiHeaderSent(), sendNow);
        _cgi.setCgiHeaderSent(_cgi.getCgiHeaderSent() + sendNow);
        return true;
    }
    return false;
}

bool    Response::sendBody(size_t chunkSize)
{
    if (_cgi.getUsingStatCgiFile())
    {
        if (_cgi.getStatCgiFilePos() < _cgi.getStatCgiFileBody().size())
        {
            size_t left = _cgi.getStatCgiFileBody().size() - _cgi.getStatCgiFilePos();
            size_t sendNow = std::min(chunkSize, left);
            chunk = _cgi.getStatCgiFileBody().substr(_cgi.getStatCgiFilePos(), sendNow);
            _cgi.setStatCgiFilePos(_cgi.getStatCgiFilePos() + sendNow);
            return _cgi.getStatCgiFilePos() < _cgi.getStatCgiFileBody().size();
        }
        else
            _cgi.setUsingStatCgiFile(false);
    }
    if (usingStaticFile)
    {
        if (staticFilePos < staticFileBody.size())
        {
            size_t left = staticFileBody.size() - staticFilePos;
            size_t sendNow = std::min(chunkSize, left);
            chunk = staticFileBody.substr(staticFilePos, sendNow);
            staticFilePos += sendNow;
            return staticFilePos < staticFileBody.size();
        }
        else
            usingStaticFile = false;
    }
    return false;
}

bool Response::sendCgiScript(size_t chunkSize)
{
    std::string body = _cgi.getcgiBody();

    if (_cgi.getCgiCL() == -1)
    {
        size_t remaining = body.size() - _cgi.getFilePos();
        size_t sendNow = std::min(chunkSize, remaining);
        if (sendNow > 0)
        {
            chunk.assign(body, _cgi.getFilePos(), sendNow);
            _cgi.setFilePos(_cgi.getFilePos() + sendNow);
            return true;
        }
    }
    else
    {
        size_t remaining = _cgi.getCgiCL() - _cgi.getFilePos();
        size_t sendNow = std::min(chunkSize, remaining);
        if (sendNow > 0)
        {
            chunk.assign(body, _cgi.getFilePos(), sendNow);
            _cgi.setFilePos(_cgi.getFilePos() + sendNow);
            return true;
        }
    }
    return false;
}


bool Response::sendFile(size_t chunkSize)
{
    char buffer[chunkSize];
    file.read(buffer, chunkSize);
    int bytesRead = file.gcount();
    if (bytesRead > 0)
    {
        chunk.assign(buffer, bytesRead);
        filePos += bytesRead;
        if (filePos >= fileSize)
            file.close();
        return true;
    }
    file.close();
    return false;
}

bool Response::getNextChunk(size_t chunkSize)
{
    chunk.clear();

    if (sendHeaders(chunkSize) == true)
        return true;
    if (sendBody(chunkSize) == true)
        return true;
    if (_cgi.getUsingCgi())
    {
        if (sendCgiScript(chunkSize) == true)
            return true;
    }
    if (file.is_open())
    {
        if (sendFile(chunkSize) == true)
            return true;
    }
    return false;
}

bool Response::checkPendingCgi(Request &req) 
{
    if (!_cgi.gethasPendingCgi())
        return false;

    int     status;
    pid_t   childPid = _cgi.getpid_1();
    int     result = waitpid(childPid, &status, WNOHANG);
    if (result > 0)
    {
        struct kevent kev;
        EV_SET(&kev, childPid, EVFILT_PROC, EV_DELETE, 0, 0, NULL);
        kevent(kq, &kev, 1, NULL, 0, NULL);
        if (WIFEXITED(status))
        {
            int exCode = WEXITSTATUS(status);
            if (exCode == 0)
            {
                if (_cgi.parseOutput() == false)
                {
                    _cgi.responseErrorcgi(500, " Internal Server Error", req);
                    _cgi.setcgistatus(CGI_ERROR);
                }
                else
                {
                    _cgi.prepareFileResponseCgi(req);
                    _cgi.setcgistatus(CGI_COMPLETED);
                }
            }
            else
            {
                _cgi.responseErrorcgi(500, " Internal Server Error", req);
                _cgi.setcgistatus(CGI_ERROR);
            }
        }
        else
        {
            _cgi.responseErrorcgi(500, " Internal Server Error", req);
            _cgi.setcgistatus(CGI_ERROR);
        }
        if (!_cgi.getinfile().empty())
            std::remove(_cgi.getinfile().c_str());
        if (!_cgi.getoutfile().empty())   
            std::remove(_cgi.getoutfile().c_str());
        _cgi.sethasPendingCgi(false);
        return true;
    }
    else if (result == -1)
    {
        struct kevent kev;
        EV_SET(&kev, childPid, EVFILT_PROC, EV_DELETE, 0, 0, NULL);
        kevent(kq, &kev, 1, NULL, 0, NULL);
        _cgi.responseErrorcgi(500, " Internal Server Error", req);
        _cgi.setcgistatus(CGI_ERROR);
        _cgi.sethasPendingCgi(false);

        if (!_cgi.getinfile().empty())
            std::remove(_cgi.getinfile().c_str());
        if (!_cgi.getoutfile().empty())   
            std::remove(_cgi.getoutfile().c_str());
        return true;
    }
    return false;
}