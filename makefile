WFLAGS=-Wall -Wextra -pedantic
OFLAGS=-g -std=c++11
OUTPUT=connect.out

all: debug_build

debug_build: ./main.cpp ./src/Board.cpp ./src/Algorithms.cpp
	g++ $(WFLAGS) $(OFLAGS) -o $(OUTPUT) $^

release_build: ./main.cpp ./src/Board.cpp ./src/Algorithms.cpp
	g++ -O3 -D NDEBUG -o $(OUTPUT) $^
