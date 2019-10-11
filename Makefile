all: main.cc
	g++ -std=c++17 -O3 -g -Wall main.cc -lncurses -o smack
