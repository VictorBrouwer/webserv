#!/usr/bin/python3

import os, sys

print("Content-type: text/html\r\n")
print("<!DOCTYPE html>")
print("<html><head><title>Hello CGI</title></head><body>")
print("<h1>Hello, CasPer VicTor JisSe CGI!</h1>")
print("<p>Your Host address is:", os.environ['REMOTE_HOST'] + "</p>")
print("<p>Your Browser is:", os.environ['HTTP_USER_AGENT'] + "</p>")
print("<p>Your Request Method is:", os.environ['REQUEST_METHOD'] + "</p>")
print("<p>Your Server Software is:", os.environ['SERVER_SOFTWARE'] + "</p>")
print("<p>Your Server Protocol is:", os.environ['SERVER_PROTOCOL'] + "</p>")
print("<p>Your Server Port is:", os.environ['SERVER_PORT'] + "</p>")

print("<h3>Body:</h3>")
print("<p>")
for line in sys.stdin:
    print(line)
print("</p>")

print("</body></html>")


# while 1:
#     pass
