CC=gcc
CFLAGS=-lm -Wall

%.o: %.c 
	$(CC) -c -o $@ $< $(CFLAGS)

WorldMaze: WorldMaze.o 
	$(CC) -o WorldMaze WorldMaze.o $(CFLAGS)

.PHONY : clean
clean : 
	-rm *.o $(objects) WorldMaze