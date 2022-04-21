CC=gcc
CFLAGS=-Wall
DEPS = mynet.h
OBJ = mynet.o image.o network.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

mynet: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

.PHONY: clean

clean:
	rm -f *.o
