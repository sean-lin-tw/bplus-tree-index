CC = gcc
CXX = g++

CFLAGS ?= -c -std=c99
CXXFLAGS ?= -c -std=c++11
TESTFLAGS ?= -std=c99 -m32

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
	./$(EXEC) < data/ProjectB_data > output.txt
	$(RM) $(OBJS)

page_size_test: src/test/test_page_size.c
	@$(CC) $(TESTFLAGS) $< -o page_size
	@./page_size
	@$(RM) page_size

slotted_page_test: src/test/test_record_manipulation.c src/src/bptree.c src/src/directory.c 
	@$(CC) $(TESTFLAGS) $< $(word 2,$^) $(word 3,$^) -lm -o slotted_page
	@./slotted_page
	@$(RM) slotted_page

bp_insert_test: src/test/test_bp_insert.c src/src/bptree.c
	@$(CC) $(TESTFLAGS) $< $(word 2,$^) -lm -o bp_insert
	@./bp_insert
	@$(RM) bp_insert

bp_delete_test: src/test/test_bp_delete.c src/src/bptree.c 
	@$(CC) $(TESTFLAGS) $< $(word 2,$^) -lm -o bp_delete
	@./bp_delete
	@$(RM) bp_delete

relation_test: src/test/test_relation_manipulation.c src/src/bptree.c src/src/directory.c src/src/relation.c
	@$(CC) $(TESTFLAGS) $< $(word 2,$^) $(word 3,$^) $(word 4,$^) -lm -o relation
	@./relation
	@$(RM) relation


.PHONY: clean
clean:
	$(RM) $(EXEC) $(OBJS) output.txt
