## webserv

    Setup phase:
        1. getaddrinfo()
        2. socket()
        3. bind()
        4. listen()
        5. setup poll() structures

    Event loop:
        1. poll()
        2. if listening socket POLLIN → accept() new client
        3. if client socket POLLIN → recv() request
        4. prepare response
        5. send() response
        6. close() client
        7. cleanup pollfd slot

# ⚡ Short Conceptual Server Flow with poll()
```c
// create listen socket
listen()

// add listen_fd to poll_list

// loop forever:
    poll(poll_list)

    for each fd thats ready:
        if fd == listen_fd:
            accept() new client
            add new client_fd to poll_list
        else:
            recv() data from client_fd
            process data
            send() response
```