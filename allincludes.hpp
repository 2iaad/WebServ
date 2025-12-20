#pragma once

#include <cstdio> 
#include <cstddef>
#include <exception>
#include <iostream>
#include <cstring>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <signal.h>
#include <poll.h>
#include <cstdlib>
#include <fcntl.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/event.h>
#include <sys/stat.h>
#include <dirent.h>
#include <set>

struct	_BoundarySettings
{
	std::string	Boundary;
	std::string	BoundaryStart;
	std::string	BoundaryEnd;
	size_t		CrlfCount;
};

enum	_BoundaryStatus
{
	None,
	GotBoundaryStart,
	GotFile,
	GotBody,
	GotBodyEnd,
	GotBoundaryEnd,
	Finished
};

struct	_ServerDetails
{
	bool		IsPortExist;
	size_t		RealPort;
	std::string	ServerHost;
	std::string	ServerPort;
};


struct	ChunkVars
{
	enum	_ChunkStatus
	{
		None, GotHexaSize, GotFullBody, Finished
	};
	_ChunkStatus	ChunkStatus;
	size_t			BodySize;
	size_t			BodySizeTmp;
	std::string		Temp;
};

void		AddToKqueue(struct kevent &event, int kq, int fd, int filter, int flags);
std::string	intToString(int n);
std::string RemoveSlashs(std::string path);
