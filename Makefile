.PHONY: all
all: ggj26

ggj26: src/main.c src/game.c src/levels.c src/episodes/episode1.c src/player.c src/enemies/enemy1.c
	gcc -o ggj26 -Isrc -I./raylib/src src/main.c src/game.c src/levels.c src/episodes/episode1.c src/player.c src/enemies/enemy1.c -L./raylib/src -lraylib -lopengl32 -lgdi32 -lwinmm
