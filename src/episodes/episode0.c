#include "../enemies/enemy.h"
#include "../types.h"
#include "episodes.h"

// Epilog map bounds in WORLD space.
// The texture used by game.c is:
//   assets/map/prolog/epilog_map.png
// We render it scaled up in-world (see EPILOG_WORLD_SCALE in game.c) to keep the
// PNG sharp. So bounds are larger than the PNG's pixel size.
#define EPILOG_W 1024.0f
#define EPILOG_H 1024.0f

void InitEpisode0(Level *level) {
    level->id = 0;

    // Spawn near bottom-left so it's easy to start collision testing from the
    // container area (H). NOTE: Episode0 world is scaled (s = 2.0f), so these
    // are world coordinates.
    level->playerSpawn = (Vector2){520.0f, 1500.0f};
    level->playerStartId = GetIdentity(ENEMY_CIVILIAN);

    // For now: win condition uses X/Y checks in game.c; set winX unused.
    level->winX = -1.0f;

    level->wallCount = 0;
    level->doorCount = 0;
    level->enemyCount = 0;

    Identity civilianIdentity = GetIdentity(ENEMY_CIVILIAN);

    level->enemies[level->enemyCount++] = (Entity){
        .position = {140.0f, 300.0f},
        .active = true,
        .isPlayer = false,
        .isEnemy = false,
        .isInteractive = true,
        .npcType = NPC_KIZ,
        .radius = 20.0f,
        .identity = civilianIdentity,
        .aiType = AI_NONE,
        .state = STATE_IDLE
    };

    level->enemies[level->enemyCount++] = (Entity){
        .position = {210.0f, 330.0f},
        .active = true,
        .isPlayer = false,
        .isEnemy = false,
        .isInteractive = true,
        .npcType = NPC_COCUK,
        .radius = 20.0f,
        .identity = civilianIdentity,
        .aiType = AI_NONE,
        .state = STATE_IDLE
    };

    level->enemies[level->enemyCount++] = (Entity){
        .position = {740.0f, 430.0f},
        .active = true,
        .isPlayer = false,
        .isEnemy = false,
        .isInteractive = true,
        .npcType = NPC_BALIKCI,
        .radius = 20.0f,
        .identity = civilianIdentity,
        .aiType = AI_NONE,
        .state = STATE_IDLE
    };

    level->enemies[level->enemyCount++] = (Entity){
        .position = {820.0f, 760.0f},
        .active = true,
        .isPlayer = false,
        .isEnemy = false,
        .isInteractive = true,
        .npcType = NPC_SIGARACI,
        .radius = 20.0f,
        .identity = civilianIdentity,
        .aiType = AI_NONE,
        .state = STATE_IDLE
    };

}
