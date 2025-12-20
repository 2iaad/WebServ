#include "responseHeader.hpp"

Response::Response()
{
    
}

Response::Response(int _clientFd, int _kq)
{
    kq = _kq;
    clientFd = _clientFd;
    filePos = 0;
    fileSize = 0;
    headerSent = 0;
    staticFilePos= 0;
    usingStaticFile = false;
    bytesSent = 0;
    _cgi.sethasPendingCgi(false);
    _cgi.setKq(kq);
}

Response::~Response()
{

}

std::string Response::getChunk()
{
    return chunk;
}

bool Response::getHasMore()
{
    return hasMore;
}

void    Response::setHasMore(bool _hasmore)
{
    hasMore = _hasmore;
}

size_t  Response::getBytesSent()
{
    return bytesSent;
}

void  Response::setBytesSent(ssize_t _bytessent)
{
    bytesSent = _bytessent;
}

long  Response::getBytesWritten()
{
    return bytesWritten;
}

void  Response::setBytesWritten(ssize_t _byteswritten)
{
    bytesWritten = _byteswritten;
}

void  Response::setHeaderSent(size_t _headerSent)
{
    headerSent = _headerSent;
}

int Response::getClientFd()
{
    return clientFd;
}

std::string handWritingError(std::string message, int statusCode)
{
    std::string _code = intToString(statusCode);
    std::string html = "<!DOCTYPE html><html lang=\"en\">";
    html += "<head><meta charset=\"UTF-8\">";
    html += "<title>Error " + _code + "</title>";
    html += "<style>";
    html += "body { background: linear-gradient(to bottom, #7C0A02, #5A0000, #7C0A02); "
            "font-family: 'Segoe UI', sans-serif; color: #f8e1e1; margin: 0; padding: 40px; "
            "display: flex; justify-content: center; align-items: center; height: 100vh; }\n";
    html += ".error-container { max-width: 600px; background: rgba(0,0,0,0.25); padding: 30px; "
            "border-radius: 12px; box-shadow: 0 8px 32px rgba(0,0,0,0.5); text-align: center; "
            "backdrop-filter: blur(10px); animation: fadeIn 0.6s ease-in-out; }\n";
    html += "h1 { font-size: 64px; color: #ffb3b3; margin-bottom: 20px; "
            "text-shadow: 0 0 10px rgba(255, 179, 179, 0.8); animation: slideDown 0.5s ease-in-out; }\n";
    html += "p { font-size: 20px; color: #ffd6d6; margin-bottom: 0; }\n";
    html += "@keyframes fadeIn { from { opacity: 0; } to { opacity: 1; } }\n";
    html += "@keyframes slideDown { from { transform: translateY(-20px); opacity: 0; } "
            "to { transform: translateY(0); opacity: 1; } }\n";
    html += "</style></head>";
    html += "<body>";
    html += "<div class=\"error-container\">";
    html += "<h1>🚫 Error " + _code + "</h1>";
    html += "<p>" + message + "</p>";
    html += "</div>";
    html += "</body></html>";

    return html;
}

std::string readFileToString(const std::string& path)
{
    std::ifstream file(path.c_str(), std::ios::binary);
    if (!file)
        return "";
    std::ostringstream contents;
    contents << file.rdbuf();
    file.close();
    return contents.str();
}

void     Response::responseError(int statusCode, std::string message, Request &req)
{
    std::string body;
    std::string	 loc = req.GetRightServer().GetRightLocation(req.GetHeaderValue("path"));
    std::string root = getInfoConfig("root", loc, req);
    std::vector<std::string> errorPage = getInfoConfigMultiple("error_page", loc, req);
    errorP = "";
    for (size_t i = 0; i + 1 < errorPage.size(); i += 2)
	{
		if (std::atoi(errorPage[i].c_str()) == statusCode)
		{
			errorPath = root;
			if (!root.empty() && root.back() != '/')
				errorPath += "/";
			errorPath += errorPage[i + 1];
			body = readFileToString(errorPath);
            if (!body.empty())
                errorP = checkContentType(2);
			break;
		}
	}
    if (body.empty())
        body = handWritingError(message, statusCode);
    staticFileBody = body;
    staticFilePos = 0;
    usingStaticFile = true;
    headers = "HTTP/1.1 " + intToString(statusCode);
    switch (statusCode)
    {
        case 403: headers += " Forbidden"; break;
        case 404: headers += " Not Found"; break;
        case 500: headers += " Internal Server Error"; break;
        case 501: headers += " Method not implemented"; break;
        case 400: headers += " Bad Request"; break;
        case 413: headers += " Content Too Large"; break;
        case 414: headers += " URI Too Long"; break;
        case 411: headers += " Length Required"; break;
        case 415: headers += " Unsupported Media Type"; break;
        case 505: headers += " HTTP Version Not Supported"; break;
        case 504: headers += " Gateway Timeout"; break;
        case 405: headers += " Method Not Allowed"; break;
    }
    headers += "\r\n";
    if (!errorP.empty())
        headers += errorP;
    else
        headers += "Content-Type: text/html\r\n";
    headers += "Content-Length: " + intToString(staticFileBody.size()) + "\r\n";
    headers += "Connection: close\r\n\r\n";
    _cgi.setCheckConnection(_close);
    headerSent = 0;
}

std::string getInfoConfig(std::string what, std::string location, Request &req)
{
    ConfigNode a = req.GetRightServer();

    std::vector<std::string> temp = a.getValuesForKey(a, what, location);
    if (!temp.empty())
        return temp[0];
	return "";
}

std::vector<std::string> getInfoConfigMultiple(std::string what, std::string location, Request &req)
{
    ConfigNode a = req.GetRightServer();

    return a.getValuesForKey(a, what, location);
}

std::string Response::checkContentType(int index)
{
    std::string exte;
    if (index == 1)
        exte = htmlFound;
    else if (index == 2)
        exte = errorPath;
    else
        exte = uri;
    size_t dotPos = exte.find_last_of(".");
    if (dotPos != std::string::npos)
    {
        std::string extension = exte.substr(dotPos);
        if (extension == ".png")   return "Content-Type: image/png\r\n";
        if (extension == ".jpg" || extension == ".jpeg") return "Content-Type: image/jpeg\r\n";
        if (extension == ".gif")   return "Content-Type: image/gif\r\n";
        if (extension == ".bmp")   return "Content-Type: image/bmp\r\n";
        if (extension == ".webp")  return "Content-Type: image/webp\r\n";
        if (extension == ".svg")   return "Content-Type: image/svg+xml\r\n";
        if (extension == ".mp4")   return "Content-Type: video/mp4\r\n";
        if (extension == ".webm")  return "Content-Type: video/webm\r\n";
        if (extension == ".ogv")   return "Content-Type: video/ogg\r\n";
        if (extension == ".avi")   return "Content-Type: video/x-msvideo\r\n";
        if (extension == ".mov")   return "Content-Type: video/quicktime\r\n";
        if (extension == ".mkv")   return "Content-Type: video/x-matroska\r\n";
        if (extension == ".mp3")   return "Content-Type: audio/mpeg\r\n";
        if (extension == ".wav")   return "Content-Type: audio/wav\r\n";
        if (extension == ".ogg")   return "Content-Type: audio/ogg\r\n";
        if (extension == ".flac")  return "Content-Type: audio/flac\r\n";
        if (extension == ".aac")   return "Content-Type: audio/aac\r\n";
        if (extension == ".pdf")   return "Content-Type: application/pdf\r\n";
        if (extension == ".zip")   return "Content-Type: application/zip\r\n";
        if (extension == ".tar")   return "Content-Type: application/x-tar\r\n";
        if (extension == ".gz")    return "Content-Type: application/gzip\r\n";
        if (extension == ".bz2")   return "Content-Type: application/x-bzip2\r\n";
        if (extension == ".7z")    return "Content-Type: application/x-7z-compressed\r\n";
        if (extension == ".rar")   return "Content-Type: application/vnd.rar\r\n";
        if (extension == ".html" || extension == ".htm") return "Content-Type: text/html\r\n";
        if (extension == ".css")   return "Content-Type: text/css\r\n";
        if (extension == ".js")    return "Content-Type: application/javascript\r\n";
        if (extension == ".json")  return "Content-Type: application/json\r\n";
        if (extension == ".xml")   return "Content-Type: application/xml\r\n";
        if (extension == ".txt")   return "Content-Type: text/plain\r\n";
        if (extension == ".csv")   return "Content-Type: text/csv\r\n";
        if (extension == ".md")    return "Content-Type: text/markdown\r\n";
        else
            return "Content-Type: text/plain\r\n";
    }
    if (access(exte.c_str(), X_OK) != 0)
        return "Content-Type: text/plain\r\n";
    else
        return "Content-Type: application/octet-stream\r\n";
}

int Response::checkLocation(Request &req, std::string meth, std::string directive)
{
    std::string	 loc = req.GetRightServer().GetRightLocation(req.GetHeaderValue("path"));
    std::vector<std::string> allowed_methods = getInfoConfigMultiple(directive, loc, req);
    if (std::find(allowed_methods.begin(), allowed_methods.end(), meth) == allowed_methods.end())
    {
        responseError(405, " Method Not Allowed", req);
        return -1;
    }
    return 0;
}

void    Response::moveToResponse(Request	&req, int e)
{
    uri = req.GetFullPath();
    method = req.GetHeaderValue("method");
    pathRequested = req.GetHeaderValue("path");

    if (method == "GET")
        getResponse(req);
    else if (method == "POST")
        postMethod(req, e);
    else if (method == "DELETE")
        deleteMethod(req);
    else
        responseError(501, " Method not implemented", req);
}
