#!/usr/bin/python3

import os

print("Content-type: text/html\r\n")
print("<!DOCTYPE html>")
print("<html><head><title>Hello CGI</title></head><body>")
print("<h1>Hello, CGI!</h1>")
print("<p>Your Host address is:", os.environ['REMOTE_HOST'] + "</p>")
print("</body></html>")
