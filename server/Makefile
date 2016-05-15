CC=g++
OPT=-Wall -ggdb -pipe

all: main.cpp server.h connection.h program_options.h net_message.h
	$(CC) $(OPT) main.cpp -o perf-research-server.exe \
	-lboost_system \
	-lboost_thread \
	-lboost_random \
	-lboost_program_options \
	-lpthread
	
tests: variable_record_header.h variable_record.h #request_parser.h
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