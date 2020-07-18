
all:libtoco.so yield server client multiThreadServer sleep signal prio

SSRC = $(wildcard  ./src/*.S)

CPPSRC = $(wildcard  ./src/*.cpp ./test/*.cpp)

OBJS =  $(patsubst %.S,%.o,$(SSRC)) $(patsubst %.cpp,%.o,$(CPPSRC))

INCLUDES = -I./test -I./include
LIBS = -L./

CFLAGS = -g -std=c++0x -fPIC -shared  -DSCH_PRIO_SIZE=8 -DSCH_STACK_SIZE=4096 -DCOR_STACK_SIZE=10240 -DSTACK_CHECK -fstack-protector-all
LFLAGS = -g $(LIBS) -ltoco -DSTACK_CHECK -fstack-protector-all

%.o:%.cpp
	g++ -c $(CFLAGS) $(INCLUDES) $< -o $@

%.o:%.S
	g++ -c $(CFLAGS)  $(INCLUDES)  $< -o $@

libtoco.so:$(OBJS)
	g++ $(CFLAGS) -o libtoco.so  ./src/*.o -lpthread -ldl 

yield:$(OBJS)
	g++ $(LFLAGS) -o yield ./test/yield.o 

client:$(OBJS)
	g++ $(LFLAGS) -o client ./test/client.o 

server:$(OBJS)
	g++ $(LFLAGS) -o server ./test/server.o 

multiThreadServer:$(OBJS)
	g++ $(LFLAGS) -o multiThreadServer ./test/multiThreadServer.o -lpthread

sleep:$(OBJS)
	g++ $(LFLAGS) -o sleep ./test/sleep.o

signal:$(OBJS)
	g++ $(LFLAGS) -o signal ./test/signal.o

prio:$(OBJS)
	g++ $(LFLAGS) -o prio ./test/prio.o
	
clean:
	rm -rf ./src/*.o ./test/*.o libtoco.so yield client server multiThreadServer sleep signal prio