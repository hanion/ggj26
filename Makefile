CC = gcc

# Common flags
CFLAGS = -I./raylib/src -Isrc

# Developer build flag (0 = players, 1 = developers)
# Usage: make DEV_MODE=1
DEV_MODE ?= 1
CFLAGS += -DDEV_MODE=$(DEV_MODE)
LDFLAGS = -L./raylib/src -lraylib

ifeq ($(OS),Windows_NT)
    # Windows
    LIBS = -lopengl32 -lgdi32 -lwinmm
else
    UNAME_S := $(shell uname -s)
    ifeq ($(UNAME_S), Darwin)
        # macOS
        LIBS = -framework CoreVideo -framework IOKit -framework Cocoa -framework GLUT -framework OpenGL
    else
        # Linux
        LIBS = -lm -ldl -lpthread -lX11
    endif
endif

SRC = src/main.c \
    src/game.c \
    src/game_context.c \
    src/levels.c \
    src/episodes/prolog.c \
    src/episodes/episode1.c \
    src/player/player.c \
    src/player/player_render.c \
    src/player/player_actions.c \
    src/ui/hud.c \
    src/enemies/enemy1.c \
    src/anim.c \
    src/episodes/episode2.c \
    src/episodes/episode3.c \
    src/enemies/enemy_factory.c \
    src/gameplay_helpers.c \
    src/masks/mask1.c \
    src/masks/mask2.c \
    src/masks/mask_manager.c \
    src/editor.c \
    src/npc/npc.c

ggj26: $(SRC)
	$(CC) -o ggj26 $(SRC) $(CFLAGS) $(LDFLAGS) $(LIBS)

clean:
	rm -f ggj26
