CC = gcc
CXX = g++

CFLAGS ?= -c -std=c99
CXXFLAGS ?= -c -std=c++11

OBJS = cli.o bptree.o directory.o relation.o

EXEC = bp_project
all: $(EXEC)

bp_project: $(OBJS)
	$(CXX) -o $@ $(OBJS)

cli.o: src/main.cpp
	$(CXX) $(CXXFLAGS) -o $@ $<

bptree.o: src/src/bptree.c
	$(CC) $(CFLAGS) -lm -o  $@ $<

directory.o: src/src/directory.c
	$(CC) $(CFLAGS) -o $@ $<

relation.o: src/src/relation.c
	$(CC) $(CFLAGS) -o $@ $<

run: $(EXEC)
	$(EXEC) < data/ProjectB_data > output.txt

.PHONY: clean
clean:
	$(RM) $(EXEC) output.txt
