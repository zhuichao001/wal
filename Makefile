all:
	g++ -g -o test_wal test.cpp
clean:
	rm -f test_wal
	rm -rf data/
