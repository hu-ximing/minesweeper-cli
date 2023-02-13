CC = gcc
CFLAGS = -Wall -Wextra -std=gnu99
LDFLAGS = -lncursesw

minesweeper: src/minesweeper.c
	$(CC) $(CFLAGS) -o minesweeper src/minesweeper.c $(LDFLAGS)

clean:
	rm -f minesweeper
