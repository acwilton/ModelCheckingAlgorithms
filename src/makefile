CC=g++
CFLAGS=-std=c++17
EXEC=buchi_driver collatz_driver

all: buchi_driver collatz_driver

buchi_driver: buchi_driver.cc *.hh
	$(CC) $(CFLAGS) $< -o $@

collatz_driver: collatz_driver.cc *.hh
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm *.o $(EXEC)
