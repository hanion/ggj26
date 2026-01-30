CC = cc
CFLAGS = -I./raylib/src
LDFLAGS = -L./raylib/src -lraylib -lm -ldl -lpthread -lX11
SRCS = src/main.c src/game.c src/levels.c src/episodes/episode1.c src/player.c src/enemies/enemy1.c

.PHONY: all clean

all: ggj26

ggj26: $(SRCS)
	$(CC) -o $@ $(SRCS) $(CFLAGS) $(LDFLAGS)

clean:
	rm -f ggj26

