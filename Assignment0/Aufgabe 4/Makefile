CC=gcc
CFLAGS=-I.
DEPS =
OBJ = strings.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

strings: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)
