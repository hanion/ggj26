.PHONY: all
all: ggj26

ggj26: src/main.c
	cc -o ggj26 src/main.c -L./raylib/src/libraylib.so -lraylib -lm
