CC=g++
CFLAGS=-std=c++17
EXEC=buchi_driver int_kripke_driver

all: buchi_driver int_kripke_driver

buchi_driver: buchi_driver.cc *.hh
	$(CC) $(CFLAGS) $< -o $@

int_kripke_driver: int_kripke_driver.cc *.hh
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm *.o $(EXEC)
