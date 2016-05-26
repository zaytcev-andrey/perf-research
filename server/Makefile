CC=g++
OPT=-Wall -ggdb -pipe

all: main.cpp program_options.h variable_record_header.h variable_record.h connection.h server.h
	$(CC) $(OPT) main.cpp -o perf-research-server.exe \
	 -lpthread \
	 -lboost_system \
	 -lboost_filesystem \
	 -lboost_thread \
	 -lboost_random \
	 -lboost_program_options
	
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