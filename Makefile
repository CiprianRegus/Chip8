CC=gcc
CFLAGS = -Wall
OBJS = cpu.o stack.o decoder.o

run: $(OBJS)
	$(CC) -o a $(CFLAGS) $(OBJS) && ./a