Operating Systems EX 4
submitting : beni tibi 208434290 ruth goldberg 322631235
-- Our program written in Clion and we using the compiler GCC --

In this EX we ipmlement a chat using pool and reactor design pattern.
We implemented a struct that will hold the handle function of the file descriptor in the poll,
also we implemented a struct of the reactor pattern that hold the number of FDs , an array of the handle struct ,
the poll and the FD number.

To run the program you need to open a terminal in the project folder and write:

-> make

-> ./reactor_server

now the server is waiting for a client to connect to him .
--We used the `telnet localhost 9034` command to create a client - any client will work

now open another terminal in the same folder and run your client on port 9034

now you can write in the clients terminal what ever you want and the server will print it .

you can open as many clients as you want.

--cntl+z or cntl+c for stoping the program.

in the zip folder you will find a run example photos from our program: server.png client1.png client2.png client3.png

