#pragma once

#include "Post.hpp"
#include "../allincludes.hpp"
#include "../pars_config/config.hpp"

#define MAX_HEADER_SIZE	8000
#define BUFFER_SIZE		80000

enum	Method
{
	GET, POST, DELETE
};

enum	ClientStatus
{
	ReadHeader, ReadBody, EndReading
};	

enum	DataType
{
	Chunked, FixedLength
};

enum	ContentType
{
	_Boundary, BinaryOrRaw
};

class	Cgi;
class	Post;
class	ConfigNode;
struct	EventContext;

class	Request
{
private:
	Post								*PostObj;
	_ServerDetails						ServerDetails;
	std::map<std::string, std::string>	ExtentionsMap;
	

	ClientStatus			Client;
	int						ClientFd;
	std::vector<ConfigNode>	Servers;
	ConfigNode				RightServer;

	Method								Method;
	DataType							DataType;
	ContentType							ContentType;
	std::map<std::string, std::string>	Headers;
	std::map<std::string, std::string>	QueryParams;
	std::string							FullSystemPath;
	std::string							UploadPath;
	std::string							Location;
	std::vector<std::string>			AllowedMethods;

	_BoundarySettings			BoundaryAttri;
	std::string					HeaderBuffer;
	std::string					BodyBuffer;
	std::string					FileExtention;
	std::string					cgiFileName;

	size_t						MaxAllowedBodySize;
	size_t						TotalBytesRead;
	size_t						ContentLength;
	bool						IsCGI;
	bool						LimitedBodySize;
	bool						KeepAlive;
	bool						RequestNotComplete;

public:
	Request(const int	&, std::vector<ConfigNode>, int &);
	~Request();

	EventContext* ctx;
	// ---------		GETTERS 	 	--------- //
	std::string							GetUploadPath() const;
	bool								GetIsCGI() const;
	bool								GetLimitedBodySize() const;
	bool								GetConnection() const;
	int									GetClientStatus() const;
	int									GetClientFd() const;
	int									GetDataType() const;
	int									GetContentType() const;
	size_t								GetContentLength() const;
	size_t								GetTotatlBytesRead() const;
	size_t    							GetMaxAllowedBodySize() const;
	std::string							GetLocation() const;
	std::string							GetFullPath() const;
	std::string							GetFileExtention();
	std::string							GetHeaderValue(std::string) const;
	std::string							GetBodyBuffer() const;
	std::string							GetHeaderBuffer() const;
	std::string							GetCgiFileName() const;
	std::map<std::string, std::string>	GetHeaders() const;
	std::map<std::string, std::string>	GetQueryParams() const;
	ConfigNode							&GetRightServer();
	_BoundarySettings					GetBoundarySettings() const;
	_ServerDetails						GetServerDetails() const;

	// ---------		SETTERS 	 	--------- //
    void	SetContext(EventContext* ctx);
	void	SetFullSystemPath(std::string	&Path);
	void	SetExtentionsMap();
	void	SetHeaderValue(std::string, std::string);
	void	SetContentLength(const size_t	Length);
	void	SetClientStatus(ClientStatus	Status);
	void	SetServerDetails();
	void	setCgiFileName(std::string _cgiFileName);
	
	// ---------	MEMBER FUNCTIONS 	--------- //

	static	void	PrintError(const std::string &Err, Request &Obj);
	int				HexaToInt(std::string	x);
	void			DecodeHexaToChar(std::string	&str);
	
	void	ReadRequestHeader();
	void	ReadFirstLine(std::string);
	void	ReadHeaders(std::string);
	void	PostRequiredHeaders();

	void	ParseHeaders();

	void	CheckFileExistance();
	void	CheckIfAllowedMethod();
	bool	CheckForCgi();
	void	HandleQuery();
	void	HandlePath();
	void	SplitURI();
	void	ParseURI();
	void	SetUpRequest();
	void	ReadBodyChunk();
	void	GetBoundaryFromHeader();
};

// ---------	HELPER FUNCTIONS 	--------- //

bool			IsHexa(char c);
void			TrimSpaces(std::string& str);
std::string		HexaToChar(std::string	Hexa);
void			PrintHeaders(std::map<std::string, std::string> Headers);
bool			ValidContentLength(const std::string& value);
bool			ValidFieldName(const std::string& name);
bool			ValidFieldValue(const std::string& value);
bool			ValidBoundary(std::string	&value);
size_t			CrlfCounter(std::string	&str, size_t pos);
void			PrintCrlfString(std::string Buffer);
std::string		RandomString();
void			Appender(std::string &Buffer, const std::string &PrevBuffer, const std::string &tmp);
std::string		PurePath(std::string &Path);