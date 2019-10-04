all: main.cc
	g++ -std=c++17 -O3 -Wall -lstdc++fs main.cc -lncurses -o smack
