#include "cgiHeader.hpp"

bool Cgi::prepareFileResponseCgi(Request &req)
{
    file.open(outFile.c_str(), std::ios::binary);
    if (!file.is_open())
        return false;

    cgiFilePos = 0;
    usingCgi = true;
    std::string httpResponse;
    httpResponse = "HTTP/1.1 " + intToString(cgiStatusCode);
    switch (cgiStatusCode)
    {
        case 100: httpResponse += " Continue"; break;
        case 101: httpResponse += " Switching Protocols"; break;
        case 102: httpResponse += " Processing"; break;
        case 103: httpResponse += " Early Hints"; break;
        case 200: httpResponse += " OK"; break;
        case 201: httpResponse += " Created"; break;
        case 202: httpResponse += " Accepted"; break;
        case 203: httpResponse += " Non-Authoritative Information"; break;
        case 204: httpResponse += " No Content"; break;
        case 205: httpResponse += " Reset Content"; break;
        case 206: httpResponse += " Partial Content"; break;
        case 207: httpResponse += " Multi-Status"; break;
        case 208: httpResponse += " Already Reported"; break;
        case 226: httpResponse += " IM Used"; break;
        case 300: httpResponse += " Multiple Choices"; break;
        case 301: httpResponse += " Moved Permanently"; break;
        case 302: httpResponse += " Moved Temporarily"; break;
        case 303: httpResponse += " See Other"; break;
        case 304: httpResponse += " Not Modified"; break;
        case 307: httpResponse += " Temporary Redirect"; break;
        case 308: httpResponse += " Permanent Redirect"; break;
        case 400: httpResponse += " Bad Request"; break;
        case 401: httpResponse += " Unauthorized"; break;
        case 402: httpResponse += " Payment Required"; break;
        case 403: httpResponse += " Forbidden"; break;
        case 404: httpResponse += " Not Found"; break;
        case 405: httpResponse += " Method Not Allowed"; break;
        case 406: httpResponse += " Not Acceptable"; break;
        case 407: httpResponse += " Proxy Authentication Required"; break;
        case 408: httpResponse += " Request Timeout"; break;
        case 409: httpResponse += " Conflict"; break;
        case 410: httpResponse += " Gone"; break;
        case 411: httpResponse += " Length Required"; break;
        case 412: httpResponse += " Precondition Failed"; break;
        case 413: httpResponse += " Payload Too Large"; break;
        case 414: httpResponse += " URI Too Long"; break;
        case 415: httpResponse += " Unsupported Media Type"; break;
        case 416: httpResponse += " Range Not Satisfiable"; break;
        case 417: httpResponse += " Expectation Failed"; break;
        case 418: httpResponse += " I'm a teapot"; break;
        case 421: httpResponse += " Misdirected Request"; break;
        case 422: httpResponse += " Unprocessable Entity"; break;
        case 423: httpResponse += " Locked"; break;
        case 424: httpResponse += " Failed Dependency"; break;
        case 425: httpResponse += " Too Early"; break;
        case 426: httpResponse += " Upgrade Required"; break;
        case 428: httpResponse += " Precondition Required"; break;
        case 429: httpResponse += " Too Many Requests"; break;
        case 431: httpResponse += " Request Header Fields Too Large"; break;
        case 451: httpResponse += " Unavailable For Legal Reasons"; break;
        case 500: httpResponse += " Internal Server Error"; break;
        case 501: httpResponse += " Not Implemented"; break;
        case 502: httpResponse += " Bad Gateway"; break;
        case 503: httpResponse += " Service Unavailable"; break;
        case 504: httpResponse += " Gateway Timeout"; break;
        case 505: httpResponse += " HTTP Version Not Supported"; break;
        case 506: httpResponse += " Variant Also Negotiates"; break;
        case 507: httpResponse += " Insufficient Storage"; break;
        case 508: httpResponse += " Loop Detected"; break;
        case 510: httpResponse += " Not Extended"; break; 
        case 511: httpResponse += " Network Authentication Required"; break;
        default : httpResponse = "HTTP/1.1 500"; httpResponse += " Internal Server Error"; break;
    }
    httpResponse += "\r\n";
    httpResponse += cgiHeader;
    if (cgiHeader.find("Content-Type") == std::string::npos)
        httpResponse += "Content-Type: text/plain\r\n";
    if (cgiHeader.find("Content-Length") == std::string::npos)
    {
        if (cgiContentLength > cgiFileSize || cgiContentLength < 0)
        {
            cgiContentLength = cgiFileSize;
            httpResponse += "Content-Length: " + intToString(cgiFileSize) + "\r\n";
        }
        else
            httpResponse += "Content-Length: " + intToString(cgiContentLength) + "\r\n";
    }
    if (cgiHeader.find("Connection") == std::string::npos)
    {
        if (req.GetHeaderValue("connection") == "keep-alive")
        {
            httpResponse += "Connection: keep-alive\r\n\r\n";
            checkConnection = keepAlive;
        }
        else
        {
            httpResponse += "Connection: close\r\n\r\n";
            checkConnection = _close;
        }
    }
    cgiHeader = httpResponse;
    cgiHeaderSent = 0;
    return true;
}

std::string handWritingErrorcgi(std::string message, int statusCode)
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

std::string readFileToStringCgi(std::string path)
{
    std::ifstream file(path.c_str(), std::ios::binary);
    if (!file)
        return "";
    std::ostringstream contents;
    contents << file.rdbuf();
    file.close();
    return contents.str();
}

void Cgi::responseErrorcgi(int statusCode, std::string message, Request &req)
{
    std::string     body;
    std::string	    loc = req.GetRightServer().GetRightLocation(req.GetHeaderValue("path"));
    std::string     root = getInfoConfigCgi("root", loc, req);
    std::vector<std::string> error_page = getInfoConfigMultipleCgi("error_page", loc, req);
    std::string errorContentTC = "";
    errorPathCgi = "";
    for (size_t i = 0; i + 1 < error_page.size(); i += 2)
	{
		if (std::atoi(error_page[i].c_str()) == statusCode)
		{
			errorPathCgi = root;
			if (!root.empty() && root.back() != '/')
				errorPathCgi += "/";
			errorPathCgi += error_page[i + 1];
			body = readFileToStringCgi(errorPathCgi);
            if (!body.empty())
                errorContentTC = checkContentTypeCgi();
			break;
		}
	}
    if (body.empty())
        body = handWritingErrorcgi(message, statusCode);
    statCgiFileBody = body;
    statCgiFilePos = 0;
    usingCgiStatFile = true;
    cgiHeader = "HTTP/1.1 " + intToString(statusCode);
    switch (statusCode)
    {
        case 403: cgiHeader += " Forbidden"; break;
        case 404: cgiHeader += " Not Found"; break;
        case 500: cgiHeader += " Internal Server Error"; break;
        case 501: cgiHeader += " Method not implemented"; break;
        case 400: cgiHeader += " Bad Request"; break;
        case 413: cgiHeader += " Content Too Large"; break;
        case 414: cgiHeader += " URI Too Long"; break;
        case 505: cgiHeader += " HTTP Version Not Supported"; break;
        case 504: cgiHeader += " Gateway Timeout"; break;
        case 405: cgiHeader += " Method Not Allowed"; break;
    }
    cgiHeader += "\r\n";
    if (!errorContentTC.empty())
        cgiHeader += errorContentTC;
    else
        cgiHeader += "Content-Type: text/html\r\n";
    cgiHeader += "Content-Length: " + intToString(body.length()) + "\r\n";
    cgiHeader += "Connection: close\r\n\r\n";
    checkConnection = _close;
    cgiHeaderSent = 0;
}