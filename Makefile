.PHONY: all
all: ggj26

ggj26: src/main.c
	gcc -o ggj26 -I./raylib/src src/main.c -L./raylib/src -lraylib -lopengl32 -lgdi32 -lwinmm
