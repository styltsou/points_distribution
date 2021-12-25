CC = mpicc
CFLAGS = -O3
RM = rm -f

EXECUTABLES = main

DEPENDENCIES = read.c utils.c quickselect.c distribute.c test.c

all: $(EXECUTABLES)

main:  main.c $(DEPENDENCIES)
	$(CC) $(CFLAGS) $^ -o $@ -lm

clean:
	$(RM) $(EXECUTABLES)