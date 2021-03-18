CC = gcc

OBJS = double_list.o link_list.o main.o
SRC = double_list.c link_list.c main.c

main:$(OBJS)
	$(CC) $(OBJS) -o main

%.o:%.c
	$(CC) -c $^ -o $@

.PHONY: clean
clean:
	rm -f *.o
	rm -f .*.sw?
