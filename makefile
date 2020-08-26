
all:libtoco.so yield server client multiThreadServer sleep signal prio mutex cond sem countDownLatch channel

SSRC = $(wildcard  ./src/*.S)

CPPSRC = $(wildcard  ./src/*.cpp ./test/*.cpp)

OBJS =  $(patsubst %.S,%.o,$(SSRC)) $(patsubst %.cpp,%.o,$(CPPSRC))

INCLUDES = -I./test -I./include
LIBS = -L./

CFLAGS = -g -std=c++0x -fPIC -shared -fstack-protector-all -lpthread\
			-DSCH_PRIO_SIZE=8\
			-DSCH_STACK_SIZE=4096\
			-DCOR_STACK_SIZE=4096\
			-DSTACK_SEPARATE\
			-DLOADBALANCE_FACTOR=95
			
LFLAGS = -g $(LIBS) -ltoco  -fstack-protector-all

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

mutex:$(OBJS)
	g++ $(LFLAGS) -o mutex ./test/mutex.o

cond:$(OBJS)
	g++ $(LFLAGS) -o cond ./test/cond.o

sem:$(OBJS)
	g++ $(LFLAGS) -o sem ./test/sem.o

countDownLatch:$(OBJS)
	g++ $(LFLAGS) -o countDownLatch ./test/countDownLatch.o

channel:$(OBJS)
	g++ $(LFLAGS) -o channel ./test/channel.o
	
clean:
	rm -rf ./src/*.o ./test/*.o libtoco.so yield client server multiThreadServer sleep signal prio mutex cond sem countDownLatch channel