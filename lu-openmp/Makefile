prog := ludec
prog_objs := ludec.o

CC := gcc
CFLAGS := -std=c99 -g -Wall -O2 -fopenmp -lm
LDFLAGS := -fopenmp

.PHONY: all clean

all: $(prog)

$(prog): $(prog_objs)
	$(CC) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

ludec.o: ludec.c

clean:
	@rm -rf *.o $(prog)
