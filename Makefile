CFLAGS= -Wall -Wextra -Werror -std=gnu++17 -g -pg -pedantic -fsanitize=thread
LDFLAGS= -pthread

SRC=main.cpp
OBJ=main.o
EXE=main
CC=g++

run: build 
	./$(EXE)

build: $(SRC)
	$(CC) $(CGLAFS) -o $(EXE) $(LDFLAGS) $(SRC)

run_with_valgring: $(EXE)
	valgrind --tool=memcheck ./$(EXE)
