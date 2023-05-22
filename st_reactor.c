#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <poll.h>
#include <pthread.h>
#include <unistd.h>
#include "st_reactor.h"



void *createReactor() {
    Reactor *reactor = malloc(sizeof(Reactor));
    if (!reactor) {
        printf("allocation failed\n");
        return NULL;
    }
    reactor->handler = (p_handler *) malloc(sizeof(p_handler));
    if (!reactor->handler) {
        printf("allocation handler failed\n");
        free(reactor);
        return NULL;
    }
    reactor->size = 1;
    reactor->counter = 0;
    reactor->fds = (struct pollfd *) malloc(sizeof(struct pollfd));
    if (!reactor->fds) {
        free(reactor->handler);
        free(reactor);
        printf("allocation fds failed\n");
        return NULL;
    }
    reactor->is_active = false;
    return reactor;
}

void stopReactor(void *this) {
    Reactor *reactor = (Reactor *) this;
    if (reactor) {
        reactor->is_active = false;
        WaitFor(reactor);
    }
}

void* reactor_thread(void* this) {
    Reactor* reactor = (Reactor*) this;
    while (reactor->is_active) {
        int numEvents = poll(reactor->fds, reactor->counter, -1);
        if (numEvents > 0) {
            for (int i = 0; i < reactor->counter; i++) {
                if (reactor->fds[i].revents & POLLIN) {
                    reactor->handler[i]->handlerFunc(reactor, reactor->fds[i].fd, reactor->handler[i]->arg);
                }
            }
        }
    }

    return NULL;
}

void startReactor(void* this) {
    Reactor* reactor = (Reactor*) this;
    if (!reactor->is_active) {
        reactor->is_active = true;
        pthread_create(&reactor->thread, NULL, reactor_thread, (void*)reactor);
    }
}


void addFd(void *this, int fd, Handler handler) {
    Reactor *reactor = (Reactor *) this;
    reactor->counter++;
    reactor->fds = (struct pollfd *) realloc(reactor->fds, reactor->counter * sizeof(struct pollfd));
    reactor->handler = (p_handler *) realloc(reactor->handler, reactor->counter * sizeof(p_handler));
    reactor->handler[reactor->counter-1] = (p_handler) malloc(sizeof(Handler));
    reactor->handler[reactor->counter-1]->handlerFunc = handler.handlerFunc;
    reactor->handler[reactor->counter-1]->arg = handler.arg;
    reactor->fds[reactor->counter-1].fd = fd;
    reactor->fds[reactor->counter-1].events = POLLIN;

}

void WaitFor(void *this) {
    Reactor *reactor = (Reactor *) this;
    if(reactor && !reactor->is_active)
        pthread_join(reactor->thread, NULL);
}
int findFD(void* this , int fd){
    Reactor * reactor = (Reactor*)this;
    for(int i = 0 ; i < reactor->counter ; i++){
        if(reactor->fds[i].fd == fd)
            return i;
    }
    return -1;
}

void deleteFD(void* this, int fd){
    Reactor * reactor = (Reactor*)this;
    int index = findFD(this,fd);
    if(index == -1){
        perror("findFD");
        exit(3);
    }
    free(reactor->handler[index]);
    for (int i = index; i < reactor->counter - 1; i++)
    {
        reactor->handler[i] = reactor->handler[i + 1];
        reactor->fds[i] = reactor->fds[i + 1];
    }
    reactor->counter--;

}
void deleteReactor(void *this){
    Reactor* reactor = (Reactor *) this;
    if (reactor) {
        if(reactor->is_active)
            stopReactor(reactor);
        if (reactor->handler) {
            for(int i = 0 ; i < reactor->counter ; i++) {
                free(reactor->handler[i]);
            }
            free(reactor->handler);
            reactor->handler = NULL;
        }

        // Free the reactor's fds array
        if (reactor->fds) {
            for(int i = 0 ; i < reactor->counter ; i++)
                close(reactor->fds[i].fd);
            free(reactor->fds);
            reactor->fds = NULL;
        }

        // Free the reactor itself
        free(reactor);

        exit(0); // Terminate the program gracefully
    }
}