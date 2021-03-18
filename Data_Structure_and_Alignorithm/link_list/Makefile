CC = gcc

OBJS = linklist.o testlinklist.o
SRC = linklist.c testlinklist.c

testlinklist:$(OBJS)
	$(CC) $(OBJS) -o testlinklist

%.o:%.c
	$(CC) -c $^ -o $@

.PHONY: clean
clean:
	rm -f *.o
	rm -f .*.sw?
