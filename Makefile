CC=gcc

run: cpu.o stack.o decoder.o
	$(CC) -o a -Wall cpu.o stack.o decoder.o && ./a