#!/usr/bin/env python3
import os

# Content-type header required by CGI
print("Content-Type: text/plain\r\n\r")

# Show current working directory
print("Current working directory:", os.getcwd())

# Try to read from a relative file
try:
    with open("data.txt", "r") as f:
        content = f.read()
    print("Read from relative path data.txt:")
    print(content)
except Exception as e:
    print("Error reading data.txt:", e)