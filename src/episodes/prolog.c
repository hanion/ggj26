// filepath: /home/noobuntu/ggj26/src/episodes/prolog.c
#include "../levels.h"
#include "../types.h"

static Texture2D texProlog;

void InitProlog(Level *level) {
    level->id = 0;
    level->playerSpawn = (Vector2){ 400, 800 };

    // Set a default Identity for the starting player
    level->playerStartId = (Identity){0};
    level->playerStartId.color = (Color){200, 200, 255, 255};
    level->playerStartId.speed = 200.0f;

    // Load map texture, with a fallback if missing
    const char *mapPath = "assets/prolog/prolog_map.png";
    if (FileExists(mapPath)) {
        texProlog = LoadTexture(mapPath);
    } else {
        TraceLog(LOG_WARNING, "Prolog map not found at %s. Using generated placeholder.", mapPath);
        Image img = GenImageColor(1920, 1080, (Color){30, 30, 35, 255});
        // Simple grid overlay
        for (int y = 0; y < 1080; y += 64) {
            for (int x = 0; x < 1920; x++) {
                ImageDrawPixel(&img, x, y, (Color){60, 60, 70, 255});
            }
        }
        for (int x = 0; x < 1920; x += 64) {
            for (int y = 0; y < 1080; y++) {
                ImageDrawPixel(&img, x, y, (Color){60, 60, 70, 255});
            }
        }
        texProlog = LoadTextureFromImage(img);
        UnloadImage(img);
    }

    level->bgs_count = 1;
    level->bgs[0].texture = texProlog;
    level->bgs[0].source = (Rectangle){0, 0, texProlog.width, texProlog.height};
    level->bgs[0].dest = (Rectangle){0, 0, texProlog.width, texProlog.height};

    level->wallCount = 0;
    level->doorCount = 0;
    level->enemyCount = 0;
}

void UnloadProlog() {
    UnloadTexture(texProlog);
}
