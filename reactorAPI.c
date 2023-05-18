#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <poll.h>

typedef void (*handler_t)(int fd);

typedef struct Reactor_t {
    bool is_active;
    int fd;
    handler_t handler;
    pthread_t thread;
} Reactor;


void *createReactor() {
    Reactor *reactor = malloc(sizeof(Reactor));
    if (!reactor) {
        printf("allocation failed\n");
        return NULL;
    }
    reactor->is_active = false;
    return reactor;
}

void stopReactor(void *this) {
    Reactor *reactor = (Reactor *) this;
    if (reactor->is_active) {
        reactor->is_active = false;
    }
}

void *reactor_thread(void *arg) {
    Reactor *reactor = (Reactor *) arg;
    struct pollfd fds;
    fds.fd = reactor->fd;
    fds.events = POLLIN;

    while (reactor->is_active) {
        int result = poll(&fds, 1, 0);  // Poll with no timeout

        if (result == -1) {
            // Handle error
        } else if (result > 0) {
            // File descriptor is ready
            if (fds.revents & POLLIN) {
                // Call the handler function
                reactor->handler(reactor->fd);
            }
        }
    }

    return NULL;
}

void startReactor(void *this) {
    Reactor *reactor = (Reactor *) this;
    if (!reactor->is_active) {
        reactor->is_active = true;
        pthread_create(&reactor->thread, NULL, reactor_thread, reactor);
    }
}

void addFd(void *this, int fd, handler_t handler);

void WaitFor(void *this) {
    Reactor *reactor = (Reactor *) this;
    pthread_join(reactor->thread, NULL);
}
