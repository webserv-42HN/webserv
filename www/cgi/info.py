#!/usr/bin/env python3

import cgi
import cgitb
import platform
import socket
from datetime import datetime

# Enable debugging
cgitb.enable()

print("Content-Type: text/html\n")
print("<html><head><title>System Info</title></head><body>")
print("<h1>Server System Information</h1>")

print("<ul>")
print(f"<li><strong>Hostname:</strong> {socket.gethostname()}</li>")
print(f"<li><strong>Platform:</strong> {platform.platform()}</li>")
print(f"<li><strong>Python Version:</strong> {platform.python_version()}</li>")
print(f"<li><strong>Current Server Time:</strong> {datetime.now()}</li>")
print("</ul>")

print("</body></html>")
