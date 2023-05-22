#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <poll.h>
#include <signal.h>
#include "st_reactor.h"

#define PORT "9034"   // Port we're listening on

Reactor * reactor= NULL;

void handleSignal(int signum) {
    deleteReactor(reactor);
}

// Get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

// Return a listening socket
int get_listener_socket(void)
{
    int listener;     // Listening socket descriptor
    int yes=1;        // For setsockopt() SO_REUSEADDR, below
    int rv;

    struct addrinfo hints, *ai, *p;

    // Get us a socket and bind it
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    if ((rv = getaddrinfo(NULL, PORT, &hints, &ai)) != 0) {
        fprintf(stderr, "selectserver: %s\n", gai_strerror(rv));
        exit(1);
    }

    for(p = ai; p != NULL; p = p->ai_next) {
        listener = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (listener < 0) {
            continue;
        }

        // Lose the pesky "address already in use" error message
        setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

        if (bind(listener, p->ai_addr, p->ai_addrlen) < 0) {
            close(listener);
            continue;
        }

        break;
    }

    freeaddrinfo(ai); // All done with this

    // If we got here, it means we didn't get bound
    if (p == NULL) {
        return -1;
    }

    // Listen
    if (listen(listener, 500) == -1) {
        return -1;
    }

    return listener;
}

// Main
int main(void)
{
    int listener;     // Listening socket descriptor

    // Set up and get a listening socket
    listener = get_listener_socket();

    if (listener == -1) {
        fprintf(stderr, "error getting listening socket\n");
        exit(1);
    }
    //create a reactor node
    reactor = createReactor();


    if(reactor == NULL){
        perror("create reactor");
        close(listener);
        exit(2);
    }
    Handler sHandler;
    sHandler.arg = NULL;
    sHandler.handlerFunc = &serverHandler;

    addFd(reactor,listener,sHandler);
    startReactor(reactor);

    signal(SIGINT, handleSignal); // Register the signal handler for SIGINT (Ctrl+C)
    signal(SIGTSTP, handleSignal); // Register the signal handler for SIGTSTP (Ctrl+z)
    while (reactor->is_active)
    {
        sleep(1);
    }
    return 0;
}

void serverHandler(Reactor* reactor, int fd, void* args){
    struct sockaddr_storage remoteaddr; // Client address
    socklen_t addrlen;
    char remoteIP[INET6_ADDRSTRLEN];
    int newfd; // Newly accepted socket descriptor

    addrlen = sizeof remoteaddr;
    newfd = accept(fd, (struct sockaddr *)&remoteaddr, &addrlen); // Accept the incoming connection
    if (newfd == -1)
    {
        perror("accept");
    }
    else
    {
        printf("server: new connection from %s on "
               "socket %d\n",
               inet_ntop(remoteaddr.ss_family,
                         get_in_addr((struct sockaddr *)&remoteaddr),
                         remoteIP, INET6_ADDRSTRLEN),newfd);

        Handler cHandler;
        cHandler.arg = NULL;
        cHandler.handlerFunc = &clientHandler;
        addFd(reactor, newfd, cHandler);
    }
}
void clientHandler(Reactor* reactor, int fd, void* args){
    char buf[BUFSIZ];
    int nbytes = recv(fd, buf, sizeof buf, 0);
    if (nbytes <= 0) {
        // Got error or connection closed by client
        if (nbytes == 0) {
            // Connection closed
            printf("pollserver: socket %d hung up\n", fd);
        } else {
            perror("recv");
        }
        close(fd); // Bye!
        deleteFD(reactor,fd);
    }
    printf("Client %d : %s",fd,buf);
}
