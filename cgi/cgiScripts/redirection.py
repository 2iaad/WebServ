print("Status: 302 Found\r")
print("Location: https://www.youtube.com/\r")
print("Content-Type: text/html\r\n\r")

print("""
<html>
  <body>
    <h1>Redirecting...</h1>
    <p>If not redirected, <a href="https://www.youtube.com/">click here</a>.</p>
  </body>
</html>
""")