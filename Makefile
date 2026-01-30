CC = cc
CFLAGS = -I./raylib/src
LDFLAGS = -L./raylib/src -lraylib -lm -ldl -lpthread -lX11

.PHONY: all clean

all: ggj26

ggj26: src/main.c
	$(CC) -o $@ $< $(CFLAGS) $(LDFLAGS)

clean:
	rm -f ggj26

