CC = gcc

OBJS = doublelist.o testdoublelist.o
SRC = doublelist.c testdoublelist.c

testdoublelist:$(OBJS)
	$(CC) $(OBJS) -o testdoublelist

%.o:%.c
	$(CC) -c $^ -o $@

.PHONY: clean
clean:
	rm -f *.o
	rm -f .*.sw?
