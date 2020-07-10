
all:libtoco.so yield server client multiThreadServer sleep signal prio

SSRC = $(wildcard  ./src/*.S)

CPPSRC = $(wildcard  ./src/*.cpp ./test/*.cpp)

OBJS =  $(patsubst %.S,%.o,$(SSRC)) $(patsubst %.cpp,%.o,$(CPPSRC))

INCLUDES = -I./test -I./include

CFLAGS = -std=c++0x -fPIC -shared  -DSCH_PRIO_SIZE=8 -DSCH_STACK_SIZE=4096 -DCOR_STACK_SIZE=4096 -DSTACK_CHECK -DSTACK_SEPARATE -fstack-protector-all

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
	g++ -o client  ./test/client.o -ltoco $(LIBS)

server:$(OBJS)
	g++ -o server  ./test/server.o  -ltoco $(LIBS)

multiThreadServer:$(OBJS)
	g++ -o multiThreadServer  ./test/multiThreadServer.o   -ltoco $(LIBS) -lpthread

sleep:$(OBJS)
	g++ -o sleep ./test/sleep.o -ltoco $(LIBS)

signal:$(OBJS)
	g++ -o signal ./test/signal.o -ltoco $(LIBS)

prio:$(OBJS)
	g++ -o prio ./test/prio.o -ltoco $(LIBS)
	
clean:
	rm -rf ./src/*.o ./test/*.o libtoco.so yield client server multiThreadServer sleep signal prio