all: main.cc
	g++ -std=c++17 -O3 main.cc -lncurses -o smack
