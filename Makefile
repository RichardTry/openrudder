CC=g++
CFLAGS=-c -Wall -I. `wx-config --cxxflags --libs`

all:
	$(CC) $(CFLAGS) main.cpp -o main.out
#all: linuxgamepaddriver

linuxgamepaddriver: driver/linuxgamepaddriver.cpp
	$(CC) $(CFLAGS) driver/linuxgamepaddriver.cpp -o driver/linuxgamepaddriver

