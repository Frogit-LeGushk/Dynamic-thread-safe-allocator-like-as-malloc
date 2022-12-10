CFLAGS= -Wall -Wextra -Werror -std=gnu++17 -g -pg -pedantic
LDFLAGS= -pthread

SRC=main.cpp
OBJ=main.o
EXE=main
CC=g++

build: $(SRC)
	$(CC) $(CGLAFS) -o $(EXE) $(LDFLAGS) $(SRC)

run: $(EXE)
	./$(EXE)

run_with_valgring: $(EXE)
	valgrind ./$(EXE)
