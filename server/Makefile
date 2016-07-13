LIB_DIR=/usr/local/lib
OPT=-Wall -ggdb -pipe -L$(LIB_DIR)
ROOT=..
INCLUDE=-I$(ROOT)/server -I$(ROOT)/common_protocol -I$(ROOT)/common_sources -I$(ROOT)/program_options
CC=g++ $(INCLUDE)

all: main.cpp
	$(CC) $(OPT) main.cpp \
	-lpthread \
	-lboost_system \
	-lboost_filesystem \
	-lboost_thread \
	-lboost_random \
	-lboost_program_options \
	-o perf-server.exe
	
tests: main_test.cpp
	$(CC) $(OPT) main_test.cpp \
	-lpthread \
	-lboost_system \
	-lboost_filesystem \
	-lboost_random \
	-lboost_thread \
	-lgtest \
	-o perf-server-tests.exe
	
clean:
	rm -rf *.o *~ *.exe