CC = gcc

OBJS = link_stack.o main.o

SRC = link_stack.c main.c link_stack.h

main: $(OBJS)
	$(CC) $(OBJS) -o main

%.o:%.c
	$(CC) -c $^ -o $@


.PHONY: clean
clean:
		rm -f *.o 
		rm -f .*.sw?	
