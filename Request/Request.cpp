#include "Request.hpp"

Request::Request(const int	&fd, std::vector<ConfigNode> _ConfigPars, int &_RealPort)
{
	ServerDetails.IsPortExist = false;
	ServerDetails.RealPort = _RealPort;
	SetExtentionsMap();
	ClientFd = fd;
	ContentType = BinaryOrRaw;
	DataType = FixedLength;
	Client = ReadHeader;
	PostObj = NULL;
	Servers = _ConfigPars;
	HeaderBuffer = "";
	BodyBuffer = "";
	ContentLength = 0;
	TotalBytesRead = 0;
	KeepAlive = false;
	LimitedBodySize = true;
	RequestNotComplete = true;
}

Request::~Request() {
	delete PostObj;
	PostObj = NULL;
}

/*	|#----------------------------------#|
	|#			 	GETTERS    			#|
	|#----------------------------------#|
*/

std::string	Request::GetUploadPath() const
{
	return this->UploadPath;
}

std::string	Request::GetLocation() const
{
	return this->Location;
}

bool	Request::GetIsCGI() const
{
	return this->IsCGI;
}

int		Request::GetClientFd() const
{
	return this->ClientFd;
}

bool	Request::GetLimitedBodySize() const
{
	return this->LimitedBodySize;
}

int		Request::GetClientStatus() const
{
	return this->Client;
}

std::string	Request::GetHeaderBuffer() const
{
	return this->HeaderBuffer;
}

std::string	Request::GetFileExtention()
{
	return this->FileExtention;
}

std::string	Request::GetCgiFileName() const
{
	return this->cgiFileName;
}

std::map<std::string, std::string>	Request::GetHeaders() const
{
	return this->Headers;
}

std::string	Request::GetHeaderValue(std::string	key) const
{
	for (std::map<std::string, std::string>::const_iterator it = Headers.begin(); it != Headers.end(); it++)
	{
		if (key == it->first)
			return it->second;
	}
	return "";
}

std::map<std::string, std::string>	Request::GetQueryParams() const
{
	return this->QueryParams;
}

bool	Request::GetConnection() const
{
	return this->KeepAlive;
}

size_t    	Request::GetMaxAllowedBodySize() const
{
	return this->MaxAllowedBodySize;
}


std::string	Request::GetFullPath() const
{
	return FullSystemPath;
}

std::string	Request::GetBodyBuffer() const
{
	return this->BodyBuffer;
}

size_t		Request::GetContentLength() const
{
	return this->ContentLength;
}

ConfigNode	&Request::GetRightServer()
{
	return this->RightServer;
}

int			Request::GetDataType() const
{
	return this->DataType;
}

_BoundarySettings	Request::GetBoundarySettings() const
{
	return this->BoundaryAttri;
}

int			Request::GetContentType() const
{
	return this->ContentType;
}

size_t				Request::GetTotatlBytesRead() const
{
	return this->TotalBytesRead;
}

_ServerDetails	Request::GetServerDetails() const
{
	return this->ServerDetails;
}

/*	|#----------------------------------#|
	|#			 	SETTERS    			#|
	|#----------------------------------#|
*/

void    Request::SetContext(EventContext* _ctx)
{
    this->ctx = _ctx;
}

void	Request::SetClientStatus(ClientStatus	Status)
{
	this->Client = Status;
}

void	Request::SetFullSystemPath(std::string	&Path)
{
	this->FullSystemPath = Path;
}

void	Request::setCgiFileName(std::string _cgiFileName)
{
	this->cgiFileName = _cgiFileName;
}

void	Request::SetHeaderValue(std::string	key, std::string NewValue)
{
	for (std::map<std::string, std::string>::iterator it = Headers.begin(); it != Headers.end(); it++)
	{
		if (key == it->first)
			return it->second = NewValue, (void)0;
	}
}


void	Request::SetExtentionsMap(void)
{
	this->ExtentionsMap["application/octet-stream"] = ".bin";
    this->ExtentionsMap["application/json"] = ".json";
    this->ExtentionsMap["application/xml"] = ".xml";
    this->ExtentionsMap["application/zip"] = ".zip";
    this->ExtentionsMap["application/gzip"] = ".gz";
    this->ExtentionsMap["application/x-tar"] = ".tar";
    this->ExtentionsMap["application/x-7z-compressed"] = ".7z";
    this->ExtentionsMap["application/pdf"] = ".pdf";
    this->ExtentionsMap["application/x-www-form-urlencoded"] = ".txt";
    this->ExtentionsMap["application/x-bzip"] = ".bz";
    this->ExtentionsMap["application/x-bzip2"] = ".bz2";
    this->ExtentionsMap["application/x-rar-compressed"] = ".rar";
    this->ExtentionsMap["application/x-msdownload"] = ".exe";
    this->ExtentionsMap["application/vnd.ms-excel"] = ".xls";
    this->ExtentionsMap["application/vnd.openxmlformats-officedocument.spreadsheetml.sheet"] = ".xlsx";
    this->ExtentionsMap["text/plain"] = ".txt";
    this->ExtentionsMap["text/html"] = ".html";
    this->ExtentionsMap["text/css"] = ".css";
    this->ExtentionsMap["text/csv"] = ".csv";
    this->ExtentionsMap["text/javascript"] = ".js";
    this->ExtentionsMap["application/javascript"] = ".js";
    this->ExtentionsMap["image/jpeg"] = ".jpg";
    this->ExtentionsMap["image/png"] = ".png";
    this->ExtentionsMap["image/gif"] = ".gif";
    this->ExtentionsMap["image/svg+xml"] = ".svg";
    this->ExtentionsMap["image/webp"] = ".webp";
    this->ExtentionsMap["image/bmp"] = ".bmp";
    this->ExtentionsMap["audio/mpeg"] = ".mp3";
    this->ExtentionsMap["audio/wav"] = ".wav";
    this->ExtentionsMap["audio/ogg"] = ".ogg";
    this->ExtentionsMap["video/mp4"] = ".mp4";
    this->ExtentionsMap["video/x-msvideo"] = ".avi";
    this->ExtentionsMap["video/webm"] = ".webm";
    this->ExtentionsMap["video/quicktime"] = ".mov";
    this->ExtentionsMap["video/x-flv"] = ".flv";
}

void	Request::SetContentLength(const size_t	Length)
{
	this->ContentLength = Length;
}

void	Request::SetServerDetails()
{
	std::string	&Host = Headers["host"];
	if (Host.empty())
		PrintError("Host Error", *this), throw 400;

	size_t		Pos = Host.find(":");
	if (Pos == std::string::npos)
	{
		ServerDetails.ServerHost = Host;
		// return ;
	}
	else
	{
		ServerDetails.IsPortExist = true;
		ServerDetails.ServerHost = Host.substr(0, Pos);
		ServerDetails.ServerPort = Host.substr(Pos + 1);

		if (ServerDetails.ServerHost.empty() || ServerDetails.ServerPort.empty())
			PrintError("Host Error", *this), throw 400;
	}
	RightServer = ConfigNode::GetServer(Servers, ServerDetails.RealPort);
}

/*	|#----------------------------------#|
	|#			TOOL FUNCTIONS    		#|
	|#----------------------------------#|
*/

void	Request::DecodeHexaToChar(std::string	&str)
{
	size_t	pos = 0;
	
	while((pos = str.find("%", pos)) != std::string::npos)
	{
		if (pos + 2 < str.size() && IsHexa(str[pos + 1]) && IsHexa(str[pos + 2]))
		{
			str.replace(pos, 3, HexaToChar(str.substr(pos + 1, 2))); // 2 letters after '%
			pos += 1; // 1 -> size d charachter
		}
		else
			PrintError("URI invalid percent-encoding", *this), throw 400;
    }
}

int		Request::HexaToInt(std::string	x)
{
	int y;
    std::stringstream stream(x);

	std::string HexaChars = "ABCDEFabcdef0123456789";
	for (size_t i = 0; i < x.size() ; i++)
	{
		if (HexaChars.find(x[i]) == std::string::npos)
			PrintError("Invalide Hexa value", *this), throw 400;
	}
    stream << x;
    stream >> std::hex >> y;

	if (y < 0)
			PrintError("Invalide Hexa value", *this), throw 400;
    return y;
}

void	Request::PrintError(const std::string	&Err, Request	&Obj)
{
	std::cerr << Err << std::endl;
	Obj.Client = EndReading;
}

/*	|#----------------------------------#|
	|#			MEMBER FUNCTIONS    	#|
	|#----------------------------------#|
*/

void	Request::CheckIfAllowedMethod()
{
	AllowedMethods = ConfigNode::getValuesForKey(RightServer, "allow_methods", Location);

	if (std::find(AllowedMethods.begin(), AllowedMethods.end(), GetHeaderValue("method")) == AllowedMethods.end())
		PrintError("Method Not Allowed", *this), throw 405;
}

bool	Request::CheckForCgi()
{
	std::string	ScriptFile;
	struct stat	stt, st;

	size_t		index = FullSystemPath.find("/cgiScripts/");
	if (index == std::string::npos)
	    return false;

	std::string	PathBeforeFolder = FullSystemPath.substr(0, index + 11);
	if (stat(PathBeforeFolder.c_str(), &stt) == 0)
	{
		if (!S_ISDIR(stt.st_mode))
			return false;
	}

	std::string PathAfterFolder = FullSystemPath.substr(index + 12);
	if (PathAfterFolder.empty())
	    return false;
	size_t FirstSlash = PathAfterFolder.find("/");
	if (FirstSlash != std::string::npos)
	    ScriptFile = PathAfterFolder.substr(0, FirstSlash);
	else
	    ScriptFile = PathAfterFolder;

	if (stat(FullSystemPath.c_str(), &st) == 0)
	{
	    if (S_ISDIR(st.st_mode))
	        return false;
	}
	size_t extPos = ScriptFile.rfind(".");
    if (extPos != std::string::npos)
    {
        std::string ext = ScriptFile.substr(extPos);
        if (ext == ".cgi" || ext == ".py" || ext == ".php")
            return true;
    }
	return false;
}

void	Request::GetBoundaryFromHeader()
{
	std::string										Boundary;
	std::map<std::string, std::string>::iterator	it = Headers.find("content-type");

	size_t	pos = it->second.find("=");
	if (pos == std::string::npos)
		PrintError("Boundary Error", *this), throw 400;

	Boundary = it->second.substr(pos + 1); // setting Boundary
	if (Boundary.size() < 1 || Boundary.size() > 69 || !ValidBoundary(Boundary))
		PrintError("Boundary Error", *this), throw 400;

	BoundaryAttri.Boundary = Boundary;
	BoundaryAttri.BoundaryStart = "--" + Boundary + "\r\n";
	BoundaryAttri.BoundaryEnd = "--" + Boundary + "--\r\n";
}

void	Request::HandleQuery()
{
	size_t			pos = 0;
	std::string		GenDelims = ":/?#[]@";
	std::string		SubDelims = "!$&\'()*+,;=";
	std::string		Reserved = GenDelims + SubDelims;
	std::string		Unreserved = "-_.~";
	std::string		Query = GetHeaderValue("query");

	for (size_t i = 0; i < Query.size(); i++) // if not alpha || digit -> check if Allowed character
	{
		if (Query[i] == '+')	Query[i] = ' ';
		if (!std::isalpha(Query[i]) && !std::isdigit(Query[i]))
		{
			if (Query[i] == '%' && i + 2 <= Query.size())
			{
				i += 2;
				continue;
			}
			if (Reserved.find(Query[i]) == std::string::npos && Unreserved.find(Query[i]) == std::string::npos)
				PrintError("Invalide Query", *this), throw 400;
		}
	}
	DecodeHexaToChar(Query);
	SetHeaderValue("query", Query);

	// ---------	Split && Save Query Params	 	--------- //

	std::istringstream	stream(Query);
	std::string			tmp, key, value;
	
	while (std::getline(stream, tmp, '&'))
	{
		pos = tmp.find("=");
		if (pos == std::string::npos)
			continue;
		key = tmp.substr(0, pos);
		value = tmp.substr(pos + 1);
		if (key.empty() || value.empty())
			continue;
		QueryParams.insert(make_pair(key, value));
	}
}

void	Request::CheckFileExistance()
{
	struct stat FileStat;
	std::string	tmp;
	IsCGI ? tmp = PurePath(FullSystemPath) : tmp = FullSystemPath;
    if (stat(tmp.c_str(), &FileStat) != 0)
        Client = EndReading, throw 404;
    if (access(tmp.c_str(), W_OK) != 0)
        PrintError("Forbiden", *this), throw 403;
}

void   Request::HandlePath()
{
	std::string		GenDelims = ":/?#[]@";
	std::string		SubDelims = "!$&\'()*+,;=";
	std::string		Reserved = GenDelims + SubDelims;
	std::string		Unreserved = "-_.~";
	std::string		UriPath = GetHeaderValue("path");
	
	UriPath = RemoveSlashs(UriPath);
	for (size_t i = 0; i < UriPath.size(); i++) // if not alpha || digit -> check if Allowed character
	{
		if (!std::isalpha(UriPath[i]) && !std::isdigit(UriPath[i]))
		{
			if (UriPath[i] == '%' && i + 2 <= UriPath.size())
			{
				i += 2;
				continue;
			}
			if (Reserved.find(UriPath[i]) == std::string::npos && Unreserved.find(UriPath[i]) == std::string::npos)
				PrintError("Invalide Path", *this), throw 400;
		}
	}
	DecodeHexaToChar(UriPath);
	SetHeaderValue("path", UriPath);

	Location = RightServer.GetRightLocation(UriPath);
	std::vector<std::string>	ConfigPath = ConfigNode::getValuesForKey(RightServer, "root", Location);
	if (ConfigPath.empty())
		FullSystemPath = "./";
	else
		FullSystemPath = ConfigPath[0];

	if (Location != "/")
		UriPath.erase(0, Location.size());

	FullSystemPath += UriPath;
	std::istringstream	stream(FullSystemPath);
	std::string			part;

	while (std::getline(stream, part, '/'))
	{
		if (part == "..")
			PrintError("Path Error", *this), throw 403; // Forbiden
	}
	IsCGI = CheckForCgi();
	CheckFileExistance();
}

void	Request::SplitURI()
{
	std::string	Path,Query;
	std::string	URI = GetHeaderValue("uri");
	
	// ---------# Parse URI #--------- //

	size_t		pos = URI.find("?");
	if (pos == std::string::npos) // Query has not been found
		Path = URI, Query = "";
	else
	{
		Path = URI.substr(0, pos);
		Query = URI.substr(pos + 1);
	}

	Headers["path"] = Path;
	Headers["query"] = Query;
}

void	Request::ParseURI()
{
	std::string URI = Headers["uri"];
	if (URI.length() > 255)
		PrintError("Request-URI too long", *this), throw 414;

	SplitURI();
	HandlePath();
	HandleQuery();
}

void	Request::ReadFirstLine(std::string	FirstLine)
{
	std::string			line;
	std::istringstream	FirstLineStream(FirstLine); // to use getline

	std::getline(FirstLineStream, line);

	std::istringstream Attributes(line);
	std::string method, URI, protocol;
	Attributes >> method >> URI >> protocol;

	if		(method == "GET")		this->Method = GET;
	else if (method == "POST")		this->Method = POST;
	else if (method == "DELETE")	this->Method = DELETE;	
	else							PrintError("Invalide request method", *this), throw 400;

	Headers["method"] = method;

	Headers["uri"] = URI;

	if (protocol.empty())
		PrintError("Bad Request", *this), throw 400;
	if (protocol != "HTTP/1.1")
		PrintError("HTTP Version Not Supported", *this), throw 505;
	Headers["protocol"] =  protocol;
}

void	Request::ReadHeaders(std::string Header)
{
	std::string			line, headerName, headerValue;
	bool				flag = false;
	std::istringstream	stream(Header);

	while (std::getline(stream, line))
	{
		if (!flag)
		{
			flag = true;
			continue;
		}
		size_t pos = line.find(": ");
		if (pos == std::string::npos)
			PrintError("Bad Request", *this), throw 400;

		headerName = line.substr(0, pos);
		if (!ValidFieldName(headerName))
			PrintError("Bad Request", *this), throw 400;

		headerValue = line.substr(pos + 2);
		TrimSpaces(headerValue);
		if (!ValidFieldValue(headerValue))
			PrintError("Bad Request", *this), throw 400;

		std::transform(headerName.begin(), headerName.end(), headerName.begin(), ::tolower);

		Headers[headerName] = headerValue;
	}
}

void	Request::PostRequiredHeaders()
{
	if (Headers.find("content-length") == Headers.end() && Headers.find("transfer-encoding") == Headers.end())
		PrintError("Missing POST headers", *this), throw 400;

	if (Headers.find("transfer-encoding") != Headers.end())
		(Headers["transfer-encoding"] == "chunked") ? DataType = Chunked : throw 501; // "Not implemented"
	if (Headers.find("content-length") != Headers.end())
	{
		if (!ValidContentLength(Headers["content-length"]))
			PrintError("Invalide Content-Length", *this), throw 400;
		ContentLength = strtod(Headers["content-length"].c_str(), NULL);
		if (DataType != Chunked)
		{
			if ((!ContentLength && TotalBytesRead) || ContentLength < TotalBytesRead)
				PrintError("Malformed Request", *this), throw 400;
			this->DataType = FixedLength;
		}
	}
	
	if (Headers.find("content-type") != Headers.end())
	{
		if (Headers["content-type"].find("multipart/form-data") != std::string::npos)
		{
			this->ContentType = _Boundary;
			GetBoundaryFromHeader();
		}
		else
		{
			if (ExtentionsMap.find(Headers["content-type"]) == ExtentionsMap.end())
				PrintError("Unsupported Media Type", *this), throw 415;
			FileExtention = ExtentionsMap[Headers["content-type"]];
			this->ContentType = BinaryOrRaw;
		}
	}
	std::vector<std::string>	PostPath = ConfigNode::getValuesForKey(RightServer, "upload_store", Location);
	if (!PostPath.empty())
		UploadPath = PostPath[0];
}

void	Request::ParseHeaders()
{
	if (Headers.find("host") == Headers.end())
		PrintError("No Host has been found!", *this), throw 400;
	if (Headers.find("content-length") != Headers.end() && !ValidContentLength(Headers.find("content-length")->second))
		PrintError("Bad Request", *this), throw 400;

	if (Headers.find("connection") != Headers.end())
		(Headers.find("connection")->second == "close") ? KeepAlive = false : KeepAlive = true;
	SetServerDetails(); // Init localhost + port
	ParseURI();
	CheckIfAllowedMethod();
	if (Method == POST)
		PostRequiredHeaders();
	else
	{
		if (BodyBuffer.size() > 0 && Headers.find("content-length") == Headers.end())
			PrintError("Length Required", *this), throw 411; // If Body exists and the method is Get or Delete
	}

	std::vector<std::string> Vec = ConfigNode::getValuesForKey(GetRightServer(), "client_max_body_size", "");
	if (Vec.empty())
		LimitedBodySize = false;
	if (LimitedBodySize)
		MaxAllowedBodySize = std::strtod(Vec[0].c_str(), NULL);

	if (LimitedBodySize && ContentLength > MaxAllowedBodySize)
		PrintError("Request Entity Too Large", *this), throw 413;
}

void Request::ReadBodyChunk()
{
    int		BytesRead = 0;
    char	buffer[BUFFER_SIZE];

	std::memset(buffer, 0, BUFFER_SIZE);
    BytesRead = read(ClientFd, buffer, BUFFER_SIZE - 1);
    if (BytesRead < 0)
        throw -1; // -1 is a flag
    if (BytesRead == 0)
	{
		Client = EndReading;
		return ;
	}
	TotalBytesRead += BytesRead;
	BodyBuffer.assign(buffer, BytesRead);
}

void	Request::ReadRequestHeader()
{
	int			BytesRead = 0;
	char		buffer[MAX_HEADER_SIZE];

	std::memset(buffer, 0, MAX_HEADER_SIZE);
	BytesRead = read(ClientFd, buffer, MAX_HEADER_SIZE - 1);
	if (BytesRead < 0)
        throw -1; // -1 is a flag
	if (BytesRead == 0)
	{
		Client = EndReading;
		if (RequestNotComplete)
			PrintError("Missing Double CRLF in headers", *this), throw 400;
	}

	HeaderBuffer.append(buffer, BytesRead);
	size_t npos = HeaderBuffer.find("\r\n\r\n");
	if (npos == std::string::npos)
	{
		if (HeaderBuffer.size() > MAX_HEADER_SIZE)
			PrintError("Header too long", *this), throw 400;
		throw -1; // didn't end yet
	}
	else
		RequestNotComplete = false, Client = ReadBody;

	BodyBuffer.assign(HeaderBuffer.substr(npos + 4));
	HeaderBuffer				= HeaderBuffer.substr(0, npos);

	TotalBytesRead += BodyBuffer.size();

	ReadFirstLine(HeaderBuffer); // First line
	ReadHeaders(HeaderBuffer); // other lines
	ParseHeaders();
}

void	Request::SetUpRequest()
{
	switch (Client)
	{
		case	ReadHeader	:	ReadRequestHeader();	break ;
		case	ReadBody	:	ReadBodyChunk();		break ;
		case	EndReading	:	throw 42 ; // continue to Get || Delete
	}
	if (Method == POST)
	{
		if (!PostObj)
			PostObj = new Post(*this);
		PostObj->HandlePost();
		throw -1;
	}
	else
	{
		Client = EndReading;
		throw 42;
	}
}