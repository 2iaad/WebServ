<div align="center">

# 🌐 Webserv — HTTP Server in C++

This project implements a fully functional HTTP server in C++ from the ground up. It supports core HTTP methods (GET, POST, DELETE), CGI execution, file uploads, auto-indexing, custom error pages, and persistent connections. The server is built using kqueue-driven non-blocking I/O for high concurrency and efficiency.

![main.cpp](./Assets/RequestFlowHttp.png)

## How HTTP communication works

When an HTTP request is sent, it follows a structured communication process called the **OSI model**, that allows data to travel between a client and a server fluently. The request is prepared, transmitted across the network, and delivered in a format the receiving side can understand and respond to.

This are the layers that a web request go through:

<img width="500px" src="https://cdn.prod.website-files.com/6850272ec839a090ed996f46/6936b2486590a6a63cf4fe92_modele_osi.webp" /> <br/>

The request is considered a packet that will be sent over network cables. For the packet to reach its target, it needs an address to go to, among many other pieces of information.
Each single layer of the **OSI** model plays an important role that provides the necessary information for the request to reach its target.

<img width="700px" src="./Assets/Tcp-http.png" />

</div>

## Code execution Flow

Here’s how I have built the server end‑to‑end using kqueue‑driven non‑blocking I/O.

### 1. Bootstrap and configuration
> Purpose: Load and parse the config, resolve ports/locations, then start the server.
- In [main.cpp](main.cpp), I start by calling `ConfigeFileFunc` which feeds into `StructConf` (in [ParseConfig/config.cpp](ParseConfig/config.cpp)) to parse the config. Once the configuration tree is ready, I kick off the server with `StartServerFunc`.
- My config parser validates blocks/directives, enforces port sanity with `CheckServerPorts`, and builds the server/location hierarchy. I lean on helpers like `RemoveSlashs`, `split_path`, and `ConfigNode::GetRightLocation` to normalize paths and resolve the best matching location.

### 2. Server setup (kqueue + listening sockets)
> Purpose: Create kqueue and listening sockets; register them to accept new clients.
- In [`HttpServer::setup_server`](AllServer/HttpServer.cpp), I create the kqueue, collect all unique ports via `GetAllPorts`, then for each port I bind/listen using `SetUpForBind` and `BindAndListen`.
- I register every listening `server_fd` for `EVFILT_READ` with `AddToKqueue` and track them in `server_fds` so the accept path is straightforward.

<div align="center">
  <img width="300px" src="./Assets/socket.png" />
</div>


### 3. Event loop and dispatch
> Purpose: Run the event loop and route each event to accept/read/write/CGI/timeout handlers.
- I drive the core loop in [`HttpServer::run`](AllServer/HttpServer.cpp) with `kevent`. When an event belongs to a `server_fd`, I accept it via `accept_new_client_fd`. Otherwise, based on filter/flags, I route to `handle_client_read` (request parsing), `handle_client_write` (sending), `handle_cgi_exit` (child process finished), or handle timeouts/errors with `handle_timeout` / `RemoveClient`.

### 4. Accepting a client and initializing per‑connection state
> Purpose: Allocate per‑connection state (request/response) and get ready to read from the client.
- On accept, I allocate per‑connection objects: a [`Request::Request`](Request/Request.cpp) seeded with the config vector and bound port, and a [`Response::Response`](Response/responseMain.cpp) tied to the client fd and kqueue.
- I keep them together in an `EventContext` and immediately register the client fd for `EVFILT_READ` via `AddToKqueue` so the request can start flowing.

<div align="center">
  <img width="700px" src="./Assets/accpet-client-noBG.png" />
</div>

### 5. Request parsing state machine
> Purpose: Parse the HTTP request (headers + optional body) and decide how it should be handled.
- In [`handle_client_read`](AllServer/HttpServer.cpp), I arm a ~30s timer then call `Request::SetUpRequest`.
- Inside [`Request::SetUpRequest`](Request/Request.cpp):
  - I read the header until `CRLFCRLF` using `Request::ReadRequestHeader`, parse the start line with `Request::ReadFirstLine`, then consume and store remaining headers via `Request::ReadHeaders`.
  - With `Request::ParseHeaders`, I validate method/host/keep‑alive, parse the URI, resolve location/root, and apply body policies (e.g., I return 411 if a body is present without `Content-Length` when it’s required).
  - If it’s a POST, I spin up the `Post` handler and process the body incrementally, yielding back to the loop when I need more data (throwing `-1` to continue reads):
    - I route content types to `Post::HandlePost`, which uses `Post::ParseBoundary` for multipart, `Post::ParseBirnaryOrRaw` for raw/binary, and `Post::ParseChunked` for chunked.
    - When writing uploads, I use `Post::WriteChunkToFile`, generating filenames and placing temp files appropriately (including CGI‑aware paths).
  - If it’s not POST, I mark reading done and yield a code (42) that indicates I’m ready to build the response.
- Back in `handle_client_read`, I catch that status and call `SetUpResponse` to prepare the response and enable `EVFILT_WRITE`.

### 6. Response routing and building
> Purpose: Build the right response (static file, dir listing, CGI, redirect, or error) with proper headers.
- In `SetUpResponse`, I map error statuses to [`Response::responseError`](Response/responseMain.cpp) (custom error pages if configured) and otherwise continue with [`Response::moveToResponse`](Response/responseMain.cpp).
- From there I dispatch by method:
  - GET → [`Response::getResponse`](Response/get.cpp)
    - I enforce `allow_methods` via `Response::checkLocation`.
    - I honor `return` directives for redirects with `Response::prepareRedirectResponse` and fall back to `Response::nonRedirect` when serving content.
    - If the path is CGI, I detect it with `_cgi.IsCgiRequest` and run it via [`Cgi::handleCgiRequest`](cgi/cgiMain.cpp); if it’s still running, I mark the response pending and defer sending. If it’s ready, I package the output via [`Cgi::prepareFileResponseCgi`](cgi/cgiResponse.cpp).
    - For static paths: directories go through `Response::servListingDiren` → `Response::generateAutoIndexOn` (listing HTML via `Response::generateListingDir`); files are served by `Response::prepareFileResponse` with content types from `Response::checkContentType`.
  - POST → I acknowledge success via [`Response::postResponse`](Response/postResponse.cpp) (after `Response::postMethod` checks) and manage keep‑alive headers appropriately.
  - DELETE → I perform the removal in [`Response::deleteResponse`](Response/delete.cpp) (after `Response::deleteMethod` checks) and return the right status, again honoring keep‑alive.
- When I render errors, I prefer configured `error_page`s; otherwise I fall back to minimal HTML via `handWritingError`, sourcing files with `readFileToString`. I always set `Connection` and `Content-Length` correctly.

### 7. Sending over kqueue and connection lifecycle
> Purpose: Stream the response efficiently and either keep the connection alive for reuse or close it.
- In [`handle_client_write`](AllServer/HttpServer.cpp), I ask [`Response::getNextChunk`](Response/sendResponse.cpp) for the next segment (headers/body/file/CGI) and stream it with `send()`, updating counters.
- When a chunk is fully sent:
  - If more remains, I continue.
  - If the response is complete:
    - For keep‑alive, I reset header counters, recreate fresh [`Request::Request`](Request/Request.cpp) and [`Response::Response`](Response/responseMain.cpp), re‑enable `EVFILT_READ`, and disable `EVFILT_WRITE` to await the next request.
    - Otherwise, I close the connection via `RemoveClient`.
- I use focused helpers for streaming: `Response::sendHeaders`, `Response::sendFile`, `Response::sendBody`, and `Response::sendCgiScript`. CGI/timer events surface in `HttpServer::run` and I handle them in `handle_cgi_exit` / `handle_cgi_timeout`. The per‑connection bookkeeping lives in `EventContext` (see [AllServer/HttpServer.hpp](AllServer/HttpServer.hpp)).

## How to test the project

- Build with [Makefile](Makefile), then run the server using my config ([config.conf](config.conf)):
  
```sh
make
./webserv config.conf
```

- Basic GET (autoindex or index page):
  
```sh
curl -v http://localhost:8080/
```

- Serve a static file:
  
```sh
curl http://localhost:8080/README.md
```

- Upload a file (multipart POST to a directory path):
  
```sh
curl -F "file=@Assets/socket.png" http://localhost:8080/
```

- Delete a file:

```sh
curl -X DELETE http://localhost:8080/socket.png
```

Tip: Your can try in a browser too (preferably Chrome), Enjoy sa7bi ;).
