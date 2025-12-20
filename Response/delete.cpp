#include "responseHeader.hpp"

std::string Response::deleteResponseSuccess(std::string message)
{
    std::string html = "<!DOCTYPE html><html lang=\"en\"><head><meta charset=\"UTF-8\">";
    html += "<title>Success</title>";
    html += "<style>";
    html += "body { background: linear-gradient(to bottom, #0b3d2e, #14532d, #0b3d2e); "
            "font-family: 'Segoe UI', sans-serif; color: #e0e0e0; margin: 0; padding: 40px; "
            "display: flex; justify-content: center; align-items: center; height: 100vh; }\n";

    html += ".success-container { max-width: 600px; background: rgba(0,0,0,0.25); padding: 30px; "
            "border-radius: 12px; box-shadow: 0 8px 32px rgba(0,0,0,0.5); text-align: center; "
            "backdrop-filter: blur(10px); animation: fadeIn 0.6s ease-in-out; }\n";

    html += "h1 { font-size: 48px; color: #a5d6a7; margin-bottom: 20px; "
            "text-shadow: 0 0 10px rgba(165, 214, 167, 0.8); animation: slideUp 0.5s ease-in-out; }\n";

    html += "@keyframes fadeIn { from { opacity: 0; } to { opacity: 1; } }\n";
    html += "@keyframes slideUp { from { transform: translateY(20px); opacity: 0; } "
            "to { transform: translateY(0); opacity: 1; } }\n";
    html += "</style></head><body>";
    html += "<div class=\"success-container\">";
    html += "<h1>✅ " + message + "</h1>";
    html += "</div></body></html>";

    return html;
}

void Response::deleteResponse(Request &req)
{
    struct stat st;

    if (stat(uri.c_str(), &st) == 0 && S_ISDIR(st.st_mode))
        responseError(403, " Permission Denied", req);
    else if (std::remove(uri.c_str()) == 0)
    {
        staticFileBody = deleteResponseSuccess("file succesefuly deleted!!");
        staticFilePos = 0;
        usingStaticFile = true;
        headers = "HTTP/1.1 200 OK\r\n";
        headers += "Content-Type: text/html\r\n";;
        headers += "Content-Length: " + intToString(staticFileBody.size()) + "\r\n";
        if (req.GetHeaderValue("connection") == "keep-alive")
        {
            headers += "Connection: keep-alive\r\n\r\n";
            _cgi.setCheckConnection(keepAlive);
        }
        else
        {
            headers += "Connection: close\r\n\r\n";
            _cgi.setCheckConnection(_close);
        }
        headerSent = 0;
    }
    else
        responseError(404, " Not Found", req);
}

void    Response::deleteMethod(Request &req)
{
    if (checkLocation(req, "DELETE", "allow_methods") == -1)
        return ;
    deleteResponse(req);
}