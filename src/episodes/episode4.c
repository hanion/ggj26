#include "../enemies/enemy.h"
#include "../levels.h"
#include "../types.h"
#include "episodes.h"

static Texture2D texBackground;

void InitEpisode4(Level *level) {
    level->id = 4;
    level->playerSpawn = (Vector2){100.0f, 320.0f};
    level->playerStartId = GetIdentity(ENEMY_CIVILIAN);
    level->winX = 2000.0f;

    level->wallCount = 0;
    level->doorCount = 0;
    level->enemyCount = 0;
    level->bgs_count = 0;

    // Load Background Texture
    if (texBackground.id == 0) {
        texBackground = LoadTexture("assets/environment/back_full2.png");
    }

    // --- BACKGROUND ---
    level->bgs[level->bgs_count++] = (Background){
        texBackground,
        (Rectangle){0, 0, texBackground.width, texBackground.height},
        (Rectangle){0, 0, texBackground.width, texBackground.height}
    };

    // --- WALLS ---
    // TODO: Add walls here
    // level->walls[level->wallCount++] = (Rectangle){x, y, w, h};

    // --- DOORS ---
    // TODO: Add doors here
    // level->doors[level->doorCount].rect = (Rectangle){x, y, w, h};
    // level->doors[level->doorCount].requiredPerm = PERM_STAFF;
    // level->doors[level->doorCount].isOpen = false;
    // level->doors[level->doorCount].animationProgress = 0.0f;
    // level->doorCount++;

    // --- ENEMIES ---
    // TODO: Add enemies here
    // level->enemies[level->enemyCount++] = InitEnemy((Vector2){x, y}, ENEMY_CIVILIAN);
}

void UnloadEpisode4() {
    if (texBackground.id != 0) {
        UnloadTexture(texBackground);
        texBackground = (Texture2D){0};
    }
}
