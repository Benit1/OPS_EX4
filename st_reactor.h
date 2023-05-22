#ifndef OPS_EX4_ST_REACTOR_H
#define OPS_EX4_ST_REACTOR_H
#include <stdbool.h>
#include <pthread.h>


typedef struct Reactor_t  Reactor;
typedef struct handler_s Handler;

typedef void (*handler_t)(Reactor*, int, void*);

typedef struct handler_s{
    handler_t handlerFunc;
    void* arg;
}Handler, *p_handler;

typedef struct Reactor_t {
    int counter;
    int size;
    bool is_active;
    p_handler *handler;
    pthread_t thread;
    struct pollfd *fds;
} Reactor;


void* createReactor();

void stopReactor(void *this);

void* reactor_thread(void *this) ;

void startReactor(void *this);

void addFd(void *this, int fd, Handler handler);

void WaitFor(void *this);

void deleteFD(void* this, int fd);
int findFD(void* this, int fd);
void serverHandler(Reactor* reactor, int fd, void* args);
void clientHandler(Reactor* reactor, int fd, void* args);
void deleteReactor(void *this);
#endif //OPS_EX4_ST_REACTOR_H
