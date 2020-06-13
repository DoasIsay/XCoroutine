
all:libtoco.so yield server client multiThreadServer

SSRC = $(wildcard  ./src/*.S)

CPPSRC = $(wildcard  ./src/*.cpp ./test/*.cpp)

OBJS =  $(patsubst %.S,%.o,$(SSRC)) $(patsubst %.cpp,%.o,$(CPPSRC))

INCLUDES = -I./test -I./include

CFLAGS = -std=c++0x -fPIC -shared

LIBS = -L./ 

%.o:%.cpp
	g++ -c $(CFLAGS) $(INCLUDES) $< -o $@

%.o:%.S
	g++ -c $(CFLAGS)  $(INCLUDES)  $< -o $@

libtoco.so:$(OBJS)
	g++ $(CFLAGS) -o libtoco.so  ./src/*.o -lpthread -ldl 

yield:$(OBJS)
	g++ -o yield ./test/yield.o -ltoco $(LIBS)

client:$(OBJS)
	g++ -o client  ./test/client.o ./test/socket.o -ltoco $(LIBS)

server:$(OBJS)
	g++ -o server  ./test/server.o  ./test/socket.o -ltoco $(LIBS)

multiThreadServer:$(OBJS)
	g++ -o multiThreadServer  ./test/multiThreadServer.o  ./test/socket.o -ltoco $(LIBS) -lpthread
	
clean:
	rm -rf ./src/*.o ./test/*.o libtoco.so yield client server multiThreadServer