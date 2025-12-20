#include "HttpServer.hpp"
#include <sys/event.h>


void	HttpServer::SetAllContexts(EventContext* ctx) { all_contexts.push_back(ctx);	}

HttpServer::HttpServer()	{ }

HttpServer::HttpServer(const HttpServer & other)	{ *this = other; }

HttpServer::~HttpServer()	{ }

std::set<int> GetAllPorts(std::vector<ConfigNode> &ConfigPars)
{
    std::set<int> allPorts;

    for (size_t i = 0; i < ConfigPars.size(); i++)
    {
        std::vector<std::string> ConfPort = ConfigPars[i].getValuesForKey(ConfigPars[i], "listen", "");

        for (size_t j = 0; j < ConfPort.size(); j++)
            allPorts.insert(atoi(ConfPort[j].c_str()));
    }

    return allPorts;
}

void SetUpForBind(struct sockaddr_in &server_addr, int port)
{
	std::memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = INADDR_ANY;
	server_addr.sin_port = htons(port);
}

int BindAndListen(int server_fd, struct sockaddr_in server_addr)
{
	int opt = 1;
	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
		return (1);
	if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0)
    {
        std::cout <<  "\033[31mFailed to bind the port \033[0m" << std::endl;
		return  (1);
    }

	if (listen(server_fd, BACKLOG) < 0)
		return  (1);
	return  0;
}

void HttpServer::AddToKqueue(struct kevent &event, int kq, intptr_t ident, int filter, int flags, void *udata, int fflags, intptr_t data)
{
	EV_SET(&event, ident, filter, flags, fflags, data, udata);
	kevent(kq, &event, 1, NULL, 0, NULL);
}

void HttpServer::setup_server(std::vector<ConfigNode> ConfigPars)
{
    std::cout << "\033[34mSetting up server...\033[0m" << std::endl;
    kq = kqueue();
    if (kq == -1)
        throw ("\033[31mFailed to create kqueue\033[0m");

    std::set<int> AllPorts = GetAllPorts(ConfigPars);

    size_t i = 0;
    for (std::set<int>::iterator it = AllPorts.begin(); it != AllPorts.end(); ++it)
    {
        int port = *it;

        int server_fd = socket(AF_INET, SOCK_STREAM, 0);
        if (server_fd == -1)
        {
            std::cout << "\033[31m[-]\033[0m \033[31mSocket creation failed for port " << port << "\033[0m\n";
            continue;
        }

        fcntl(server_fd, F_SETFD, FD_CLOEXEC);
        fcntl(server_fd, F_SETFL, O_NONBLOCK);

        struct sockaddr_in server_addr;
        SetUpForBind(server_addr, port);
        if (BindAndListen(server_fd, server_addr) == 1)
        {
            i++;
            close(server_fd);
            continue;
        }

        struct kevent event;
        EV_SET(&event, server_fd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
	    if(kevent(kq, &event, 1, NULL, 0, NULL) == -1)
            throw ("\033[31mkevent failed\033[0m");
        server_fds.push_back(server_fd);

        std::cout << "\033[32m[+]\033[0m \033[32mServers listening on port " << port << "\033[0m" << std::endl;
    }
    if (i == AllPorts.size())
        throw "all ports are not working gl next time";
    std::cout << "------------------------------------------------------------------------------" << std::endl;
}

std::string ip_port_to_string(const struct sockaddr_in &addr)
{
    uint32_t ip_raw = ntohl(addr.sin_addr.s_addr);
    int port = ntohs(addr.sin_port);
    std::stringstream ss;
    ss << ((ip_raw >> 24) & 0xFF) << "."
       << ((ip_raw >> 16) & 0xFF) << "."
       << ((ip_raw >> 8) & 0xFF) << "."
       << (ip_raw & 0xFF) << ":"
       << port;

    return ss.str();
}

void HttpServer::accept_new_client_fd(int server_fd, std::vector<ConfigNode> ConfigPars)
{
    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);
    int client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &addr_len);

    if (client_fd < 0)
    {
        if (errno != EAGAIN && errno != EWOULDBLOCK)
            std::cout << "\033[31m[-]\033[0m \033[31mAccept failed on server_fd " << server_fd << ": " << std::strerror(errno) << "\033[0m" << std::endl;
        return ;
    }

    fcntl(client_fd, F_SETFD, FD_CLOEXEC);
    fcntl(client_fd, F_SETFL, O_NONBLOCK);

    std::string client_ip = ip_port_to_string(client_addr);
    
    struct sockaddr_in server_addr;
    socklen_t server_addr_len = sizeof(server_addr);
    getsockname(client_fd, (struct sockaddr*)&server_addr, &server_addr_len);

    std::string server_ip = ip_port_to_string(server_addr);

    std::cout << "\033[32m[+]\033[0m \033[32mClient " << client_ip  << " connected to server at " << server_ip << "\033[0m\n" << std::endl;

    int server_port = ntohs(server_addr.sin_port);

    Request* req = new Request(client_fd, ConfigPars, server_port);
    Response* res = new Response(client_fd, kq);

	EventContext* ctx = new EventContext;
	ctx->ident = client_fd;
	ctx->cgi_pid = 0;
	ctx->req = req;
	ctx->res = res;
	ctx->is_cgi = false;

	SetAllContexts(ctx);
    struct kevent kev;
	AddToKqueue(kev, kq, client_fd, EVFILT_READ, EV_ADD | EV_ENABLE, ctx, 0, NULL);

	AddToKqueue(kev, kq, client_fd, EVFILT_WRITE, EV_ADD | EV_DISABLE, ctx, 0, NULL);

    EV_SET(&kev, client_fd, EVFILT_TIMER, EV_ADD | EV_ENABLE, NOTE_SECONDS, 30, ctx);
	kevent(kq, &kev, 1, NULL, 0, NULL);
    return ;
}

void	SetUpResponse(EventContext* ctx, Response * res, Request	*Request, int &e)
{
	switch (e)
	{
		case 500: res->responseError(500, " Internal Server Error", *Request); return;
		case 505: res->responseError(505, " HTTP Version Not Supported", *Request); return;
		case 501: res->responseError(501, " Not Implemented", *Request); return;
		case 400: res->responseError(400, " Bad Request", *Request); return;
		case 403: res->responseError(403, " Forbidden", *Request); return;
		case 404: res->responseError(404, " Not found", *Request); return;
		case 405: res->responseError(405, " Method Not Allowed", *Request); return;
        case 411: res->responseError(411, " Length Required", *Request); return;
		case 413: res->responseError(413, " Content Too Large", *Request); return;
		case 414: res->responseError(414, " URI Too Long", *Request); return;
		case 415: res->responseError(415, " Unsupported Media Type", *Request); return;
	}
	Request->SetContext(ctx);
	res->moveToResponse(*Request, e);
}


void HttpServer::RemoveClient(int client_fd)
{
    std::cout << "\033[31m[-]\033[0m \033[31mClose Client FD : " << client_fd << "\033[0m" << std::endl;

    struct kevent kev;

    EV_SET(&kev, client_fd, EVFILT_READ, EV_DELETE, 0, 0, NULL);
    kevent(kq, &kev, 1, NULL, 0, NULL);
    EV_SET(&kev, client_fd, EVFILT_WRITE, EV_DELETE, 0, 0, NULL);
    kevent(kq, &kev, 1, NULL, 0, NULL);
    EV_SET(&kev, client_fd, EVFILT_TIMER, EV_DELETE, 0, 0, NULL);
    kevent(kq, &kev, 1, NULL, 0, NULL);

    if (client_fd >= 0)
        close(client_fd);

    for (std::vector<EventContext*>::iterator it = all_contexts.begin(); it != all_contexts.end(); ++it)
    {
        EventContext* ctx = *it;
        if (ctx->ident == client_fd)
        {
            // - Deregister any process/timer we recorded in ctx->registered_procs
            if (!ctx->registered_procs.empty())
            {
                for (size_t pi = 0; pi < ctx->registered_procs.size(); ++pi)
                {
                    pid_t p = ctx->registered_procs[pi];
                    if (p <= 0) continue;
                    EV_SET(&kev, p, EVFILT_PROC, EV_DELETE, 0, 0, NULL);
                    kevent(kq, &kev, 1, NULL, 0, NULL);
                    EV_SET(&kev, p, EVFILT_TIMER, EV_DELETE, 0, 0, NULL);
                    kevent(kq, &kev, 1, NULL, 0, NULL);
                }
                ctx->registered_procs.clear();
            }
            else
            {
                pid_t proc_pid = ctx->cgi_pid;
                if (proc_pid == 0 && ctx->res)
                {
                    pid_t rpid = ctx->res->_cgi.getpid_1();
                    if (rpid > 0) proc_pid = rpid;
                }
                if (proc_pid > 0)
                {
                    EV_SET(&kev, proc_pid, EVFILT_PROC, EV_DELETE, 0, 0, NULL);
                    kevent(kq, &kev, 1, NULL, 0, NULL);
                    EV_SET(&kev, proc_pid, EVFILT_TIMER, EV_DELETE, 0, 0, NULL);
                    kevent(kq, &kev, 1, NULL, 0, NULL);
                }
            }

            ctx->marked_for_deletion = true;
            pending_deletions.push_back(ctx);

            all_contexts.erase(it);
            break;
        }
    }
}


void HttpServer::RemoveReqRes(int client_fd)
{
    std::cout << "\033[31mReset Request/Response for FD : " << client_fd << "\033[0m" << std::endl;

    for (std::vector<EventContext*>::iterator it = all_contexts.begin(); it != all_contexts.end(); ++it)
    {
        EventContext* ctx = *it;
        if (ctx->ident == client_fd)
        {
            if (ctx->req)
            {
                delete ctx->req;
                ctx->req = NULL;
            }
            if (ctx->res)
            {
                delete ctx->res;
                ctx->res = NULL;
            }
            ctx->is_cgi = false;
            return;
        }
    }
}

void HttpServer::handle_client_write(EventContext* ctx, Request * request, Response * response, std::vector<ConfigNode> ConfigPars)
{
    struct kevent kev;

    EV_SET(&kev, request->GetClientFd(), EVFILT_TIMER, EV_ADD | EV_ENABLE, NOTE_SECONDS, 30, ctx);
    kevent(kq, &kev, 1, NULL, 0, NULL);
	if (response->getChunk().empty() || response->getBytesSent() >= response->getChunk().size())
	{
		response->setHasMore(response->getNextChunk(8192));
			response->setBytesSent(0);
	}
	if (!response->getChunk().empty())
	{
		response->setBytesWritten(send(ctx->ident, response->getChunk().c_str() + response->getBytesSent(), response->getChunk().size() - response->getBytesSent(), 0));
		if (response->getBytesWritten() < 0)
			return;
		else if (response->getBytesWritten() == 0)
		{
			RemoveClient(ctx->ident);
			return;
		}
		response->setBytesSent(response->getBytesSent() + response->getBytesWritten());
		if (response->getBytesSent() < response->getChunk().size())
			return;
	}

	if (!response->getHasMore())
	{
		response->setHeaderSent(0);
		response->_cgi.setCgiHeaderSent(0);
		
		if (response->_cgi.getCheckConnection() == keepAlive)
		{
			int fd = ctx->ident;
			int server_port = ctx->req ? ctx->req->GetServerDetails().RealPort : -1;

			RemoveReqRes(fd);

			Request* new_request = new Request(fd, ConfigPars, server_port);
			Response* new_response = new Response(fd, kq);

			ctx->req = new_request;
			ctx->res = new_response;
			ctx->is_cgi = 0;
			ctx->cgi_pid = 0;
			struct kevent ev;
            EV_SET(&ev, fd, EVFILT_TIMER, EV_ADD | EV_ENABLE, NOTE_SECONDS, 30, ctx);
            kevent(kq, &ev, 1, NULL, 0, NULL);
			
			AddToKqueue(ev, kq, fd, EVFILT_WRITE, EV_DISABLE, ctx, 0, 0);
			AddToKqueue(ev, kq, fd, EVFILT_READ, EV_ADD | EV_ENABLE, ctx, 0, 0);
		}
		else 
		{
			response->_cgi.setCheckConnection(_Empty);
			RemoveClient(ctx->ident);
		}
	}

}

void HttpServer::handle_client_read(EventContext* ctx, Request * request, Response * response)
{
        struct kevent kev;

        EV_SET(&kev, request->GetClientFd(), EVFILT_TIMER, EV_ADD | EV_ENABLE, NOTE_SECONDS, 30, ctx);
        kevent(kq, &kev, 1, NULL, 0, NULL);
		try { request->SetUpRequest(); }
		catch (int	&e)
		{
			if (request->GetClientStatus() != EndReading || e == -1)
				return;

			SetUpResponse(ctx, response, request, e);
			if (response->_cgi.gethasPendingCgi())
				return;
			else
			{
				struct kevent ev;
				AddToKqueue(ev, kq, ctx->ident, EVFILT_WRITE, EV_ADD | EV_ENABLE, ctx, 0, 0);
				AddToKqueue(ev, kq, ctx->ident, EVFILT_READ, EV_DISABLE, ctx, 0, 0);
			}
		}
}

void HttpServer::handle_cgi_exit(EventContext* ctx, Request * request, Response * response)
{
	pid_t exitedPid = ctx->cgi_pid;
    if (ctx->cgi_pid == 0 || ctx->is_cgi == false || ctx->cgi_timed_out)
        return;
	if (ctx->cgi_pid != 0 && ctx->is_cgi && response->_cgi.gethasPendingCgi() && response->_cgi.getpid_1() == exitedPid)
	{
		if (response->checkPendingCgi(*request))
		{
			pid_t pid = response->_cgi.getpid_1();
			struct kevent kev;
			EV_SET(&kev, pid, EVFILT_PROC, EV_DELETE, 0, 0, NULL);
			kevent(kq, &kev, 1, NULL, 0, NULL);

			struct kevent ev;
			int client_fd = response->getClientFd();
			AddToKqueue(ev, kq, client_fd, EVFILT_WRITE, EV_ADD | EV_ENABLE, ctx, 0, 0);
			AddToKqueue(ev, kq, client_fd, EVFILT_READ, EV_DISABLE, ctx, 0, 0);

            EV_SET(&kev, response->getClientFd(), EVFILT_TIMER, EV_ADD | EV_ENABLE, NOTE_SECONDS, 30, ctx);
            kevent(kq, &kev, 1, NULL, 0, NULL);
			ctx->is_cgi = false;
		}
		response->_cgi.sethasPendingCgi(false);
	}
}

void HttpServer::handle_cgi_timeout(EventContext* ctx, Request & request, Response & response)
{
    if (ctx->cgi_pid != 0)
    {
        pid_t pid = response._cgi.getpid_1();
        struct kevent kev;
        ctx->cgi_pid = 0;
        ctx->is_cgi = false;
        ctx->cgi_timed_out = true;
        response._cgi.sethasPendingCgi(false);

        EV_SET(&kev, pid, EVFILT_PROC, EV_DELETE, 0, 0, NULL);
        kevent(kq, &kev, 1, NULL, 0, NULL);
        EV_SET(&kev, pid, EVFILT_TIMER, EV_DELETE, 0, 0, NULL);
        kevent(kq, &kev, 1, NULL, 0, NULL);
        kill(pid, SIGKILL);
        int status;
        while ((waitpid(pid, &status, WNOHANG) >= 0))
            kill(pid, SIGKILL);
        response._cgi.responseErrorcgi(504, " Gateway Timeout", request);
        response._cgi.setcgistatus(CGI_ERROR);
        response._cgi.sethasPendingCgi(false);
        if (!response._cgi.getinfile().empty())
            std::remove(response._cgi.getinfile().c_str());
        if (!response._cgi.getoutfile().empty())
            std::remove(response._cgi.getoutfile().c_str());
        struct kevent ev;
        int client_fd = response.getClientFd();
        AddToKqueue(ev, kq, client_fd, EVFILT_WRITE, EV_ADD | EV_ENABLE, ctx, 0, 0);
        AddToKqueue(ev, kq, client_fd, EVFILT_READ, EV_DISABLE, ctx, 0, 0);
        EV_SET(&ev, response.getClientFd(), EVFILT_TIMER, EV_ADD | EV_ENABLE, NOTE_SECONDS, 30, ctx);
        kevent(kq, &ev, 1, NULL, 0, NULL);
        return;
    }
}

void HttpServer::handle_timeout(EventContext* ctx, Request & request)
{

	if (!ctx || !ctx->req || !ctx->res)
	{
		if (ctx) RemoveClient(ctx->ident);
		return;
	}

	if (ctx->cgi_pid != 0 && ctx->is_cgi == true)
    {
        struct kevent kev;

        EV_SET(&kev, request.GetClientFd(), EVFILT_TIMER, EV_DELETE, 0, 0, NULL);
        kevent(kq, &kev, 1, NULL, 0, NULL);
		return;
    }
    RemoveClient(ctx->ident);
}

void HttpServer::run(std::vector<ConfigNode> ConfigPars)
{
    while (true)
    {
        int nev = kevent(kq, NULL, 0, events, BACKLOG, NULL);
        if (nev < 0)
        {
            if (errno == EINTR) continue;
            std::cout << "\033[31mkevent error: " << std::strerror(errno) << "\033[0m" << std::endl;
            continue;
        }

        for (int i = 0; i < nev; ++i)
        {
            bool isServerSocket = false;
            for (size_t j = 0; j < server_fds.size(); ++j)
            {
                if (server_fds[j] == (int)events[i].ident)
                {
                    accept_new_client_fd(static_cast<int>(events[i].ident), ConfigPars);
                    isServerSocket = true;
                    break;
                }
            }
            if (isServerSocket) continue;

            int filter = events[i].filter;
            int flags = events[i].flags;
            unsigned int fflags = events[i].fflags;

            EventContext* ctx = static_cast<EventContext*>(events[i].udata);

            if (ctx == NULL)
                continue;
			if (ctx->marked_for_deletion)
				continue;

            if (filter == EVFILT_PROC)
            {
                if (fflags & NOTE_EXIT)
                {
                    if (ctx->cgi_timed_out)
                        continue;
                    handle_cgi_exit(ctx, ctx->req, ctx->res);
                    continue;
                }
            }

            if ((filter == EVFILT_READ || filter == EVFILT_WRITE) && (flags & EV_EOF))
            {
                RemoveClient(ctx->ident);
                continue;
            }

            if (flags & EV_ERROR)
            {
                RemoveClient(ctx->ident);
                continue;
            }

            if (ctx->is_cgi == false)
            {
                if (filter == EVFILT_READ)
                    handle_client_read(ctx, ctx->req, ctx->res);
                else if (filter == EVFILT_WRITE)
                    handle_client_write(ctx, ctx->req, ctx->res, ConfigPars);
                else if (filter == EVFILT_TIMER)
                    handle_timeout(ctx, *ctx->req);
            }
            else
            {
                if (filter == EVFILT_TIMER)
                    handle_cgi_timeout(ctx, *ctx->req, *ctx->res);
            }
        }

		if (!pending_deletions.empty())
		{
			for (std::vector<EventContext*>::size_type k = 0; k < pending_deletions.size(); ++k)
			{
				EventContext* to_delete = pending_deletions[k];

				struct kevent kev;
				if (!to_delete->registered_procs.empty())
				{
					for (size_t pi = 0; pi < to_delete->registered_procs.size(); ++pi)
					{
						pid_t p = to_delete->registered_procs[pi];
						if (p <= 0) continue;
						EV_SET(&kev, p, EVFILT_PROC, EV_DELETE, 0, 0, NULL);
						kevent(kq, &kev, 1, NULL, 0, NULL);
						EV_SET(&kev, p, EVFILT_TIMER, EV_DELETE, 0, 0, NULL);
						kevent(kq, &kev, 1, NULL, 0, NULL);
					}
					to_delete->registered_procs.clear();
				}
				else
				{
					pid_t proc_pid = to_delete->cgi_pid;
					if (proc_pid == 0 && to_delete->res)
						proc_pid = to_delete->res->_cgi.getpid_1();

					if (proc_pid > 0)
					{
						EV_SET(&kev, proc_pid, EVFILT_PROC, EV_DELETE, 0, 0, NULL);
						kevent(kq, &kev, 1, NULL, 0, NULL);
						EV_SET(&kev, proc_pid, EVFILT_TIMER, EV_DELETE, 0, 0, NULL);
						kevent(kq, &kev, 1, NULL, 0, NULL);
					}
				}

				if (to_delete->req) { delete to_delete->req; to_delete->req = NULL; }
				if (to_delete->res) { delete to_delete->res; to_delete->res = NULL; }
				delete to_delete;
			}
			pending_deletions.clear();
		}
	}


}
