CC=gcc


build: main.o
	$(CC) -o a main.o

run: main.o
	$(CC) -o a main.o && ./a