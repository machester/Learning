CC = gcc

OBJS = seq_que.o main.o

SRC = seq_que.c main.c

main: $(OBJS)
	$(CC) $(OBJS) -o main

%.o:%.c
	$(CC) -c $^ -o $@


.PHONY: clean
clean:
		rm -f *.o 
		rm -f .*.sw?	
