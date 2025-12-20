#!/usr/bin/env python3
import os


print("Content-Type: text/plain\r\n\r")

vars_to_check = [
    "REQUEST_METHOD",
    "SCRIPT_NAME",
    "SCRIPT_FILENAME",
    "QUERY_STRING",
    "SERVER_PROTOCOL",
    "GATEWAY_INTERFACE",
    "SERVER_SOFTWARE",
    "PATH_INFO",
    "CONTENT_TYPE",
    "CONTENT_LENGTH",
    "SERVER_NAME",
    "SERVER_PORT"
]

# Print each variable and its value
for var in vars_to_check:
    print(f"{var} = {os.environ.get(var, '')}")
