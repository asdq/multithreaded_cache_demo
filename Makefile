CXX = g++ -std=c++11
CXXFLAGS = -Wall -march=native -O2
LDFLAGS = $(shell mysql_config --libs) -lmysqlcppconn 

all: database test
	
database: records.sql
	mysql < $^
	
test: test.cpp db_cache.cpp mysql_client.cpp
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(LDFLAGS) -o $@ $^
	
threadcheck: test
	valgrind --tool=helgrind ./test
	
memcheck: test
	valgrind --leak-check=full ./test
	
profile: test
	valgrind --tool=callgrind ./test
	
clean:
	rm -f test callgrind.out.*
	