#!/usr/bin/env python3

import base64
import fcntl
import hashlib
import json
import os
import re
import secrets
import sys
from datetime import datetime, timezone
from urllib.parse import parse_qs


# ---------------- Send pages ----------------
def send(content, set_cookie_value=None):
    """Send HTTP response with optional Set-Cookie header and content."""
    body_bytes = content.encode("utf-8")
    print("Content-Type: text/html; charset=utf-8\r")
    if set_cookie_value is not None:
        # Only servers send Set-Cookie; never send a "Cookie" header in responses
        # Add secure-ish attributes as appropriate for your environment (add 'Secure' if HTTPS)
        print(f"Set-Cookie: token={set_cookie_value}; Path=/; HttpOnly; SameSite=Strict\r")
    print(f"Content-Length: {len(body_bytes)}\r\n\r")
    # print()
    # Write bytes to avoid accidental encoding issues with large bodies
    sys.stdout.flush()
    sys.stdout.buffer.write(body_bytes)

# ---------------- HTML pages ----------------
def get_login_page():
    return """<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="utf-8">
<title>Login</title>
<style>
    body {
        background: linear-gradient(to bottom, #0b3d2e, #14532d, #0b3d2e);
        font-family: 'Segoe UI', sans-serif;
        color: #e0e0e0;
        padding: 40px;
        margin: 0;
    }
    .container {
        max-width: 500px;
        margin: auto;
        background: rgba(0, 0, 0, 0.25);
        padding: 30px;
        border-radius: 12px;
        box-shadow: 0 8px 32px rgba(0, 0, 0, 0.5);
        backdrop-filter: blur(10px);
        animation: fadeIn 0.6s ease-in-out;
        text-align: center;
    }
    h1 {
        color: #a5d6a7;
        margin-bottom: 20px;
        text-shadow: 0 0 10px rgba(165, 214, 167, 0.8);
        animation: slideDown 0.5s ease-in-out;
    }
    label {
        display: block;
        margin: 15px 0 10px;
        font-weight: bold;
        color: #c8e6c9;
    }
    input[type="text"],
    input[type="password"] {
        width: 100%;
        padding: 10px;
        border-radius: 6px;
        border: none;
        margin-top: 5px;
        box-sizing: border-box;
        background: rgba(255, 255, 255, 0.1);
        color: #fff;
        font-size: 1em;
    }
    input[type="submit"] {
        margin-top: 20px;
        padding: 10px 20px;
        background: #43a047;
        border: none;
        border-radius: 6px;
        color: #fff;
        font-weight: bold;
        cursor: pointer;
        transition: background 0.3s ease;
        font-size: 1em;
    }
    input[type="submit"]:hover {
        background: #4caf50;
    }

    @keyframes fadeIn {
        from { opacity: 0; }
        to { opacity: 1; }
    }
    @keyframes slideDown {
        from { transform: translateY(-20px); opacity: 0; }
        to { transform: translateY(0); opacity: 1; }
    }
</style>
</head>
<body>
    <div class="container">
        <h1>🔐 Login</h1>
        <form action="" method="POST">
            <label>Username:
                <input type="text" name="username" required>
            </label>
            <label>Password:
                <input type="password" name="password" required>
            </label>
            <input type="submit" value="Login">
        </form>
    </div>
</body>
</html>
"""



def styled_message_page(title: str, message: str):
    return f"""<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="utf-8">
<title>{title}</title>
<style>
    body {{
        background: linear-gradient(to bottom, #0b3d2e, #14532d, #0b3d2e);
        font-family: 'Segoe UI', sans-serif;
        color: #e0e0e0;
        padding: 40px;
        margin: 0;
    }}
    .container {{
        max-width: 600px;
        margin: auto;
        background: rgba(0, 0, 0, 0.25);
        padding: 30px;
        border-radius: 12px;
        box-shadow: 0 8px 32px rgba(0, 0, 0, 0.5);
        backdrop-filter: blur(10px);
        text-align: center;
        animation: fadeIn 0.6s ease-in-out;
    }}
    h1 {{
        color: #a5d6a7;
        margin-bottom: 20px;
        text-shadow: 0 0 10px rgba(165, 214, 167, 0.8);
        animation: slideDown 0.5s ease-in-out;
    }}
    p {{
        font-size: 1.2em;
        margin-top: 10px;
        color: #c8e6c9;
    }}
    a {{
        display: inline-block;
        margin-top: 20px;
        padding: 10px 20px;
        background: #2e7d32;
        color: #fff;
        text-decoration: none;
        border-radius: 6px;
        transition: background 0.3s ease;
        font-weight: bold;
    }}
    a:hover {{
        background: #388e3c;
    }}
    @keyframes fadeIn {{
        from {{ opacity: 0; }}
        to {{ opacity: 1; }}
    }}
    @keyframes slideDown {{
        from {{ transform: translateY(-20px); opacity: 0; }}
        to {{ transform: translateY(0); opacity: 1; }}
    }}
</style>
</head>
<body>
    <div class="container">
        <h1>{title}</h1>
        <p>{message}</p>
    </div>
</body>
</html>"""


# ---------------- Post data ----------------
def get_post_body():
    """Read exact bytes from stdin as given by CONTENT_LENGTH."""
    clen = os.environ.get('CONTENT_LENGTH', '')
    try:
        length = int(clen) if clen else 0
    except ValueError:
        length = 0
    if length > 0:
        return sys.stdin.read(length)
    return ''

# ------------------ Hash password ---------------
def hash_password(password: str) -> str:
    return hashlib.sha256(password.encode('utf-8')).hexdigest()

def base64url_encode(data):
    if isinstance(data, str):
        data = data.encode('utf-8')
    return base64.urlsafe_b64encode(data).decode('utf-8').rstrip('=')

def base64url_decode(data):
    padding = '=' * (-len(data) % 4)
    return base64.urlsafe_b64decode(data + padding)

def token_create(username):
    """Create a simple unsigned token containing the username (base64url of JSON)."""
    payload = {'username': str(username)}
    return base64url_encode(json.dumps(payload))

def store_token(username, password_hash, token_value):
    """Append 'username,password_hash,token' to CookiesData.txt with a lock."""
    with open('CookiesData.txt', 'a') as f:
        fcntl.flock(f.fileno(), fcntl.LOCK_EX)
        f.write(f"{username},{password_hash},{token_value}\n")
        fcntl.flock(f.fileno(), fcntl.LOCK_UN)

def file_read_lines_locked():
    try:
        with open("CookiesData.txt", 'r') as f:
            fcntl.flock(f.fileno(), fcntl.LOCK_SH)
            lines = f.readlines()
            fcntl.flock(f.fileno(), fcntl.LOCK_UN)
            return [ln.rstrip('\n') for ln in lines]
    except FileNotFoundError:
        return []

def token_parse(token):
    """Return payload dict or None on error."""
    try:
        decoded = base64url_decode(token)
        return json.loads(decoded)
    except Exception:
        return None

def validate_token(token):
    """Return line 'username,password_hash,token' if token exists and parses, else None."""
    payload = token_parse(token)
    if not payload or 'username' not in payload:
        return None
    for ln in file_read_lines_locked():
        # Strict token match in the 3rd CSV field
        parts = ln.split(",", 2)
        if len(parts) == 3 and parts[2] == token:
            return ln
    return None

def extract_token_from_cookie(cookie_header):
    """Extract 'token' value from HTTP_COOKIE header."""
    if not cookie_header:
        return None
    # Split on ';' and strip spaces
    for part in cookie_header.split(';'):
        part = part.strip()
        if part.startswith('token='):
            return part[len('token='):]
    return None

def find_user_by_username(username):
    """Return (username, password_hash, token) or None."""
    for ln in file_read_lines_locked():
        parts = ln.split(",", 2)
        if len(parts) == 3 and parts[0] == username:
            return tuple(parts)
    return None

def is_alphanumeric(s: str) -> bool:
    """Return True if the string contains only alphanumeric characters."""
    return s.isalnum()

def main():
    cookie_header = os.environ.get("HTTP_COOKIE", "")
    method = os.environ.get("REQUEST_METHOD", "GET").upper()

    if method == "POST":
        raw_body = get_post_body()
        parsed = parse_qs(raw_body, keep_blank_values=True)
        username = parsed.get('username', [''])[0]
        password = parsed.get('password', [''])[0]

        if not username or not password:
            send(styled_message_page("Login Failed", "Missing username or password"))
            return
        if not (is_alphanumeric(username)):
            send(styled_message_page("Invalid Username", "Username must be alphanumeric only"))
            return
        found = find_user_by_username(username)
        if found:
            # Existing user
            userna, stored_pwd_hash, existing_token = found
            if hash_password(password) == stored_pwd_hash:
                # Valid login; refresh cookie (optional) by re-setting it
                send(styled_message_page("Welcome Back", f"Welcome back, <strong>{userna}</strong>!"), set_cookie_value=existing_token)
            else:
                send(styled_message_page("Incorrect Password", f"Incorrect password for user <strong>{userna}</strong>"))
            return
        else:
            # New user: create account and set cookie
            token = token_create(username)
            store_token(username, hash_password(password), token)
            send(styled_message_page("Login Successful", f"Welcome, new user <strong>{username}</strong>!"), set_cookie_value=token)
            return

    # GET
    token = extract_token_from_cookie(cookie_header)
    valid_line = validate_token(token) if token else None
    if valid_line:
        user, pswd_hash, tok = valid_line.split(",", 2)
        send(styled_message_page("Welcome", f"Welcome back, <strong>{user}</strong>!"))
        return

    login_page = get_login_page()
    send(login_page)

if __name__ == "__main__":
    main()