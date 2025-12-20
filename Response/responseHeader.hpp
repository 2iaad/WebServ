#pragma once

#include "../cgi/cgiHeader.hpp"

class Response
{
    private:
        int                            _E;
        int                            kq;
        std::string                    uri;
        std::ifstream                  file;
        std::string                    index;
        std::string                    chunk;
        std::string                    errorP;
        std::string                    method;
        size_t                         filePos;
        bool                           hasMore;
        std::string                    headers;
        size_t                         fileSize;
        int                            clientFd;
        std::string                    errorPath;
        ssize_t                        bytesSent;
        std::string                    htmlFound;
        size_t                         headerSent;
        std::string                    autoIndexOn;
        ssize_t                        bytesWritten;
        std::string                    finalResponse;
        std::string                    pathRequested;
        size_t                         staticFilePos;
        std::string                    staticFileBody;
        bool                           usingStaticFile;

    public :
        Cgi                            _cgi;
        Response();
        Response(int _clientFd, int _kq);
        ~Response();
        
        bool    sendBody(size_t chunkSize);
        bool    sendFile(size_t chunkSize);
        bool    sendHeaders(size_t chunkSize);
        bool    sendCgiScript(size_t chunkSize);


        std::string                    getChunk();
        bool                           getHasMore();
        int                            getClientFd();
        size_t                         getBytesSent();
        long                           getBytesWritten();
        void                           setHasMore(bool _hasmore);
        void                           getResponse(Request	&req);
        void                           deleteMethod(Request &req);
        std::string                    checkContentType(int index);
        void                           deleteResponse(Request &req);
        bool                           checkPendingCgi(Request &req);
        void                           servListingDiren(Request	&req);
        bool                           getNextChunk(size_t chunkSize);
        void                           postMethod(Request &req, int e);
        void                           setBytesSent(ssize_t _bytessent);
        void                           setHeaderSent(size_t _headerSent);
        void                           postResponse(Request &req, int e);
        std::string                    generateListingDir(Request	&req);
        bool                           generateAutoIndexOn(Request	&req);
        void                           moveToResponse(Request	&req, int e);
        void                           setBytesWritten(ssize_t _byteswritten);
        std::string                    postResponseSuccess(std::string message);
        std::string                    deleteResponseSuccess(std::string message);
        void                           responseError(int statusCode, std::string message, Request &req);
        void                           nonRedirect(std::string redirectUrl, Request &req, int statusCode);
        int                            checkLocation(Request &req, std::string meth, std::string directive);
        void                           prepareRedirectResponse(std::vector<std::string> redirect, Request &req);
        int                            prepareFileResponse(std::string filepath, std::string contentType, Request &req);
};


std::string                            frontPage(std::string uri);
std::string                            getInfoConfig(std::string what, std::string location, Request &req);
std::vector<std::string>               getInfoConfigMultiple(std::string what, std::string location, Request &req);