# ToyCoroutine
a simple c++ coroutine lib, just for fun, you can treat it as a toy

g++ src/* -I./include -ldl -fPIC -shared -o libtoycoroutine.so

g++ server.cpp socket.cpp -I../include -L./ -ltoycoroutine  -o server

g++ client.cpp -o client

