#include "cgiHeader.hpp"

std::string Cgi::frontPageCgi(std::string _uri)
{
    std::string html;
    html += "<!DOCTYPE html>\n<html>\n<head>\n";
    html += "<meta charset=\"UTF-8\">\n<title>Index of " + _uri + "</title>\n";
    html += "<style>\n"
        "body { background: linear-gradient(to bottom, #0b3d2e, #14532d, #0b3d2e); font-family: 'Segoe UI', sans-serif; color: #e0e0e0; padding: 40px; margin: 0; }\n"
        ".container { max-width: 950px; margin: auto; background: rgba(0,0,0,0.25); padding: 30px; border-radius: 12px; box-shadow: 0 8px 32px rgba(0,0,0,0.5); backdrop-filter: blur(10px); animation: fadeIn 0.6s ease-in-out; }\n"
        "h1 { color: #a5d6a7; margin-bottom: 20px; text-shadow: 0 0 10px rgba(165, 214, 167, 0.8); animation: slideDown 0.5s ease-in-out; }\n"
        "ul { list-style-type: none; padding: 0; margin: 0; }\n"
        "li { margin: 10px 0; padding: 12px; border-radius: 6px; background: rgba(255,255,255,0.05); display: flex; align-items: center; transform: translateX(-10px); opacity: 0; animation: slideIn 0.4s forwards; }\n"
        "li:hover { background: rgba(76,175,80,0.25); transform: scale(1.02); transition: all 0.3s ease; }\n"
        "a { text-decoration: none; color: #80cbc4; font-weight: bold; flex-grow: 1; transition: color 0.3s ease; }\n"
        "a:hover { color: #b2dfdb; }\n"
        ".icon { margin-right: 12px; font-size: 20px; }\n"

        /* Upload section */
        ".upload-section { margin-top: 30px; padding: 25px; background: rgba(0,0,0,0.25); border-radius: 12px; box-shadow: 0 4px 20px rgba(0,0,0,0.4); text-align: center; color: #c8e6c9; animation: fadeIn 1s ease-in-out; }\n"
        ".upload-section h2 { margin-bottom: 15px; font-size: 1.3rem; }\n"
        ".file-input { display: inline-block; padding: 10px 20px; background: #2e7d32; color: #fff; border-radius: 6px; cursor: pointer; transition: background 0.3s ease; }\n"
        ".file-input:hover { background: #388e3c; }\n"
        "input[type=file] { display: none; }\n"
        "input[type=submit] { margin-top: 15px; padding: 10px 20px; background: #43a047; border: none; border-radius: 6px; color: #fff; font-weight: bold; cursor: pointer; transition: background 0.3s ease; }\n"
        "input[type=submit]:hover { background: #4caf50; }\n"

        /* Animations */
        "@keyframes fadeIn { from { opacity: 0; } to { opacity: 1; } }\n"
        "@keyframes slideIn { to { opacity: 1; transform: translateX(0); } }\n"
        "@keyframes slideDown { from { transform: translateY(-20px); opacity: 0; } to { transform: translateY(0); opacity: 1; } }\n"
        "</style>\n</head>\n<body>\n";

    html += "<div class=\"container\">\n";
    html += "<h1>📂 Index of " + _uri + "</h1>\n<ul>\n";
    return html;
}

std::string Cgi::generateListingDirCgi(Request &req)
{
    std::string pathRequest = req.GetHeaderValue("path");
    std::string _uri = req.GetFullPath();
    DIR *dirCheck = opendir(_uri.c_str());
    if (!dirCheck)
        return "";

    std::string html = frontPageCgi(_uri);
    struct dirent *dir;
    while ((dir = readdir(dirCheck)) != NULL)
    {
        std::string name = dir->d_name;
        if (name[0] == '.' && name != "." && name != "..")
            continue;

        std::string path = pathRequest;
        if (pathRequest[pathRequest.size() - 1] != '/')
            path += '/';
        path += name;
        std::string icon = (dir->d_type == DT_DIR) ? "📁" : "📄";
        html += "<li><span class=\"icon\">" + icon + "</span><a href=\"" + path + "\">" + name + "</a></li>\n";
    }
    closedir(dirCheck);
    html += "</ul>\n";
    html += "<div class=\"upload-section\">\n";
    html += "<h2>⬆️ Upload a File</h2>\n";
    html += "<form action=\"" + pathRequest + "\" method=\"post\" enctype=\"multipart/form-data\">\n";
    html += "<label class=\"file-input\">Choose File<input type=\"file\" name=\"file\"></label><br>\n";
    html += "<input type=\"submit\" value=\"Upload\">\n";
    html += "</form>\n</div>\n";
    html += "</div>\n</body>\n</html>\n";
    return html;
}

bool Cgi::generateAutoIndexOnCgi(Request &req)
{
    statCgiFileBody = generateListingDirCgi(req);
    if (statCgiFileBody.empty())
    {
        responseErrorcgi(403, "Forbidden", req);
        return false;
    }
    statCgiFilePos = 0;
    usingCgiStatFile = true;
    cgiHeader = "HTTP/1.1 200 OK\r\n";
    cgiHeader += "Content-Length: " + intToString(statCgiFileBody.size()) + "\r\n";
    cgiHeader += "Content-Type: text/html\r\n";
    if (req.GetHeaderValue("connection") == "keep-alive")
    {
        cgiHeader += "Connection: keep-alive\r\n\r\n";
        checkConnection = keepAlive;
    }
    else
    {
        cgiHeader += "Connection: close\r\n\r\n";
        checkConnection = _close;
    }
    cgiHeaderSent = 0;
    return true;
}

int    Cgi::servListingDirenCgi(Request &req, std::string uri)
{
    std::string	loc = req.GetRightServer().GetRightLocation(req.GetHeaderValue("path"));
    std::string autoIndexOn = getInfoConfigCgi("autoindex", loc, req);
    std::string index = getInfoConfigCgi("index", loc, req);

    struct stat st;
    stat(uri.c_str(), &st);
    if (!S_ISDIR(st.st_mode))
        return 0;
    if (!index.empty())
    {
        std::string htmlFound = uri;
        if (uri.back() != '/')
            htmlFound += "/";
        htmlFound += index;
        struct stat st;
        if (stat(htmlFound.c_str(), &st) == 0)
        {
            if (S_ISDIR(st.st_mode) || access(htmlFound.c_str(), R_OK) != 0)
            {
                if (autoIndexOn == "on")
                    generateAutoIndexOnCgi(req);
                else
                    responseErrorcgi(403, " Forbidden", req);
                return -1;
            }
            cgiHeader = "";
            int checkCode = IsCgiRequest(htmlFound, req);
            if (checkCode == 1)
            {
                req.SetFullSystemPath(htmlFound);
                handleCgiRequest(req);
                req.SetFullSystemPath(uri);
                if (getcgistatus() == CGI_RUNNING)
                {
                    hasPendingCgi = true;
                    return -1;
                }
                hasPendingCgi = false;
                return -1;
            }
            else if (checkCode == 0)
                return 0;
        }
        else if (autoIndexOn == "on")
            generateAutoIndexOnCgi(req);
        else
            responseErrorcgi(403, " Forbidden", req);
    }
    else if (autoIndexOn == "on")
        generateAutoIndexOnCgi(req);
    else
        responseErrorcgi(403, " Forbidden", req);
    return -1;
}