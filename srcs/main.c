#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netdb.h> // Required for struct addrinfo and related functions
#include <poll.h>
#include <errno.h>
#include <arpa/inet.h>
#include <netinet/in.h>
// #include <ostream>

int main(int argc, char *argv[])
{
    struct addrinfo hints;
    struct addrinfo *result, *rp;
    int sfd, s;
    struct sockaddr_storage peer_addr;
    socklen_t peer_addr_len;
    ssize_t nread;
    char buf[BUF_SIZE];

    if (argc != 2) {
        fprintf(stderr, "Usage: %s port\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;        /* Allow IPv4 or IPv6 */
    hints.ai_socktype = SOCK_STREAM;    /* Provides sequenced, reliable, two-way  connection-based 
                                        byte  streams. An out-of-band  data  transmission 
                                        mechanism may be supported. */
    hints.ai_flags = AI_PASSIVE;        /* For wildcard IP address */
    hints.ai_protocol = 0;              /* Any protocol */
    hints.ai_canonname = NULL;
    hints.ai_addr = NULL;
    hints.ai_next = NULL;

    s = getaddrinfo(NULL, argv[1], &hints, &result);
    if (s != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
        exit(EXIT_FAILURE);
    }

    /* getaddrinfo() returns a list of address structures.
    Try each address until we successfully bind(2).
    If socket(2) (or bind(2)) fails, we (close the socket
    and) try the next address. */

    for (rp = result; rp != NULL; rp = rp->ai_next) {
        sfd = socket(rp->ai_family, rp->ai_socktype,
            rp->ai_protocol);
        if (sfd == -1)
            continue;
        int flags = fcntl(sfd, F_GETFL, 0);
        fcntl(sfd, F_SETFL, O_NONBLOCK);
        if (bind(sfd, rp->ai_addr, rp->ai_addrlen) == 0)
            break;                  /* Success */
        close(sfd);
    }
    if (rp == NULL) {
        perror("Could not bind\n");
        exit(EXIT_FAILURE);
    }
    freeaddrinfo(result);
    if (listen(sfd, SOMAXCONN) == -1){
        perror("listen");
        exit(EXIT_FAILURE);
    }
    struct pollfd fds[MAX_CLIENTS + 1];
    int nfds = 1;

    // Initialize the first pollfd structure for the listening socket
    fds[0].fd = sfd;
    fds[0].events = POLLIN;

    // Initialize remaining pollfd structures
    for (int i = 1; i <= MAX_CLIENTS; i++) {
        fds[i].fd = -1;
    }

    printf("Server is listening on port %s...\n", argv[1]);
    
    for(;;) {
        int poll_count = poll(fds, nfds, -1);
        
        if (poll_count == -1) {
            perror("poll");
            exit(EXIT_FAILURE);
        }

        // Check for new connections
        if (fds[0].revents & POLLIN) {
            peer_addr_len = sizeof(peer_addr);
            int cfd = accept(sfd, (struct sockaddr *)&peer_addr, &peer_addr_len);
            
            if (cfd != -1) {
                int i;
                for (i = 1; i <= MAX_CLIENTS; i++) {
                    if (fds[i].fd == -1) {
                        fds[i].fd = cfd;
                        fds[i].events = POLLIN;
                        nfds = (i + 1 > nfds) ? i + 1 : nfds;
                        break;
                    }
                }
                if (i > MAX_CLIENTS) {
                    printf("Too many clients\n");
                    close(cfd);
                }
            }
        }

        // Check client sockets
        for (int i = 1; i < nfds; i++) {
            if (fds[i].fd != -1 && fds[i].revents & POLLIN) {
                ssize_t nread = recv(fds[i].fd, buf, BUF_SIZE - 1, 0);
                
                if (nread > 0) {
                    buf[nread] = '\0';
                    printf("Received request from client %d:\n%s\n", i, buf);
                    const char *response = 
                        "HTTP/1.1 200 OK\r\n"
                        "Content-Type: text/html\r\n"
                        "Content-Length: 11\r\n"
                        "\r\n"
                        "Hello World";
                    send(fds[i].fd, response, strlen(response), 0);
                    close(fds[i].fd);
                    fds[i].fd = -1;
                } else if (nread == 0 || errno != EAGAIN) {
                    close(fds[i].fd);
                    fds[i].fd = -1;
                }
            }
        }
    }
    return 0;
}