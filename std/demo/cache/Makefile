CC = clang++
CXX = clang++
CFLAGS = -Wall -march=native -O2
CPPFLAGS = -std=c++11 $(CFLAGS)
LDFLAGS = $(shell mysql_config --libs) -lmysqlcppconn 

test: test.o db_cache.o mysql_client.o
test.o : test.cpp db_cache.h mysql_client.h Makefile
mysql_client.o : mysql_client.cpp mysql_client.h Makefile
db_cache.o : db_cache.cpp db_cache.h Makefile

clean:
	rm -f test *.o

threadcheck: test
	valgrind --tool=helgrind ./test
	
memcheck: test
	valgrind ./test