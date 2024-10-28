CC=gcc
CFLAGS=-I.
DEPS =
OBJ = mult.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

mult: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)
