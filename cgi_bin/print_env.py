#!/usr/bin/python3

import os

print("Content-type: text/html\n")
print("<html><head><title>Hello CGI</title></head><body>")
print("<h1>Hello, CGI!</h1>")
print("<p>Your IP address is:", os.environ['PATH'] + "</p>")
print("</body></html>")
