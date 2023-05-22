CC = gcc
RPATH := -Wl,-rpath=./
LIBRARY := libst_reactor.so
FLAGS = -Wall -g


all: st_reactor react_server

react_server: react_server.c st_reactor.h $(LIBRARY)
	$(CC) $(RPATH) -o react_server react_server.c -L. -lst_reactor -lpthread


st_reactor: st_reactor.c st_reactor.h
	$(CC) -c -fPIC st_reactor.c -o st_reactor.o
	$(CC) -shared -o libst_reactor.so st_reactor.o

clean:
	rm -rf *.o react_server *.so

