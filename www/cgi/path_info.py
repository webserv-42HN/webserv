#!/usr/bin/env python3

import os
import sys

# Print HTTP headers first
print("Content-Type: text/html\n")

# Start HTML output
print("<html>")
print("<head><title>CGI PATH_INFO Test</title>")
print("<style>")
print("body { font-family: Arial, sans-serif; margin: 20px; }")
print("h1 { color: #333; }")
print("h2 { color: #0066cc; margin-top: 20px; }")
print(".env-var { background: #f5f5f5; padding: 10px; margin: 5px 0; border-radius: 5px; }")
print(".path-segment { color: #cc0000; font-weight: bold; }")
print("</style></head>")
print("<body>")

# Main heading
print("<h1>CGI PATH_INFO Test</h1>")

# Display PATH_INFO
path_info = os.environ.get('PATH_INFO', '')
print("<h2>PATH_INFO:</h2>")
print(f"<div class='env-var'>{path_info}</div>")

# Parse and display path segments if PATH_INFO exists
if path_info:
    print("<h2>Path Segments:</h2>")
    # Split the path into segments and display each one
    segments = [s for s in path_info.split('/') if s]
    if segments:
        print("<ul>")
        for i, segment in enumerate(segments):
            print(f"<li>Segment {i+1}: <span class='path-segment'>{segment}</span></li>")
        print("</ul>")
    else:
        print("<p>No path segments found.</p>")

# Display other relevant CGI environment variables
print("<h2>Other CGI Environment Variables:</h2>")
relevant_vars = [
    'REQUEST_METHOD',
    'QUERY_STRING',
    'SCRIPT_NAME',
    'SERVER_NAME',
    'SERVER_PORT',
    'SERVER_PROTOCOL',
    'REMOTE_ADDR'
]

print("<ul>")
for var in relevant_vars:
    value = os.environ.get(var, '')
    print(f"<li><strong>{var}:</strong> <div class='env-var'>{value}</div></li>")
print("</ul>")

print("</body></html>")