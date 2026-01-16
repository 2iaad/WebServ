# Contributing to WebServ

Thanks for your interest in improving my C++ HTTP server.

- Project overview: non-blocking kqueue-driven HTTP server with GET/POST/DELETE, CGI, uploads, autoindex, and custom errors. Key files:
  - [main.cpp](main.cpp)
  - [Makefile](Makefile)
  - [config.conf](config.conf)
  - [AllServer/HttpServer.cpp](AllServer/HttpServer.cpp)
  - [Request/Request.cpp](Request/Request.cpp)
  - [Response/responseMain.cpp](Response/responseMain.cpp)
  - [Response/get.cpp](Response/get.cpp)
  - [Response/postResponse.cpp](Response/postResponse.cpp)
  - [Response/delete.cpp](Response/delete.cpp)
  - [cgi/cgiMain.cpp](cgi/cgiMain.cpp)
  - [cgi/cgiScripts](cgi/cgiScripts)

## Prerequisites
- C++ toolchain with C++98 support.
- macOS/BSD-compatible kqueue.
- curl for manual testing.

## Build & Run
- Use [Makefile](Makefile):
```sh
make
./webserv config.conf
```
- Quick tests are in [README.md](README.md).

## Coding Standards
- C++98, compile flags: -Wall -Wextra -Werror (see [Makefile](Makefile)).
- Keep code consistent with existing style; prefer small helpers and clear state machines.
- Preserve non-blocking I/O and kqueue patterns in [AllServer/HttpServer.cpp](AllServer/HttpServer.cpp).

## Configuration & CGI
- Update server/location directives in [config.conf](config.conf).
- Place CGI scripts under [cgi/cgiScripts](cgi/cgiScripts). Ensure executable permissions.

## Branches, Commits, PRs
- Branch naming: feature/<name>, fix/<name>, docs/<name>.
- Commit messages: type(scope): short description.
- PR checklist:
  - Builds cleanly with `make`.
  - Basic curl tests pass (GET/POST upload/DELETE).
  - No regressions in autoindex/CGI/error pages.
  - Minimal, focused diffs.

## Reporting Issues
- Include steps to reproduce, expected vs actual behavior.
- Link relevant files (e.g., [Request/Request.cpp](Request/Request.cpp), [Response/get.cpp](Response/get.cpp)) when applicable.

Enjoy ajmi!