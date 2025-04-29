// <METHOD> <URL> <HTTP_VERSION> ->Request line
// <Header-Field-1>: <value>
// <Header-Field-2>: <value>
// ...
// <Body (optional)>

// GET /index.html HTTP/1.1
// Host: www.example.com
// User-Agent: Mozilla/5.0
// Accept: text/html

// Although the line terminator for the start-line and fields is the sequence CRLF(\r\n), 
// a recipient MAY recognize a single LF as a line terminator and ignore any preceding CR.

#include "../includes/Request.hpp"
