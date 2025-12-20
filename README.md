# 🌐 Webserv — HTTP Server in C++

![Language](https://img.shields.io/badge/Language-C%2B%2B98-blue)
![Status](https://img.shields.io/badge/Status-Finished-success)
![42](https://img.shields.io/badge/42-Network-black)
![Project](https://img.shields.io/badge/Project-Webserv-informational)

> A lightweight HTTP/1.1 web server written from scratch in **C++**, developed as part of the **42 Network curriculum**.

---

## 📌 Overview

**Webserv** is a custom HTTP server inspired by **Nginx** behavior.  
The goal of this project is to understand how a real web server works internally by implementing core networking and HTTP features **without using external frameworks**.

The server handles multiple clients concurrently using **non-blocking I/O** and supports configurable virtual servers through a configuration file.

---

## ✨ Features

- ✅ HTTP/1.1 compliant
- ✅ Non-blocking sockets (`poll`)
- ✅ Multiple clients handled concurrently
- ✅ Custom configuration file parsing
- ✅ Virtual servers (host & port based)
- ✅ HTTP methods:
  - `GET`
  - `POST`
  - `DELETE`
- ✅ Static file serving
- ✅ File upload handling
- ✅ CGI execution (e.g. PHP)
- ✅ Custom error pages
- ✅ Directory listing (autoindex)
- ✅ Chunked transfer encoding
- ✅ Graceful connection handling

---

## 🧠 Key Concepts Implemented

- Low-level networking (sockets, bind, listen, accept)
- Multiplexing with `poll()`
- HTTP request parsing
- HTTP response construction
- Process management for CGI
- Configuration-driven architecture
- Resource and memory safety in C++


