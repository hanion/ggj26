#include "../types.h"
#include "episodes.h"
#include "../enemies/enemy.h" // GetIdentity(), ENEMY_CIVILIAN

// Epilog map bounds in WORLD space.
// The texture used by game.c is:
//   assets/map/prolog/epilog_map.png
// We render it scaled up in-world (see EPILOG_WORLD_SCALE in game.c) to keep the
// PNG sharp. So bounds are larger than the PNG's pixel size.
// NOTE: These constants are reserved for future collision/boundary checking
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

    // --- Epilog NPCs ---
    // Must match EPILOG_WORLD_SCALE in game.c
    const float s = 2.0f;

    // Girl (kiz) - interactive NPC
    {
        Entity e = (Entity){0};
        e.position = (Vector2){140.0f * s, 300.0f * s};
        e.active = true;
        e.isPlayer = false;
        e.isEnemy = false;
        e.isInteractive = true;
        e.npcType = NPC_KIZ;
        e.radius = 20.0f;
        e.aiType = AI_NONE;
        e.state = STATE_IDLE;
        if (level->enemyCount < MAX_ENEMIES) {
            level->enemies[level->enemyCount++] = e;
        }
    }

    {
        Entity e = (Entity){0};
        e.position = (Vector2){740.0f * s, 430.0f * s};
        e.active = true;
        e.isPlayer = false;
        e.isEnemy = false;
        e.isInteractive = true;
        e.npcType = NPC_BALIKCI;
        e.radius = 20.0f;
        e.aiType = AI_NONE;
        e.state = STATE_IDLE;
        if (level->enemyCount < MAX_ENEMIES) {
            level->enemies[level->enemyCount++] = e;
        }
    }

    {
        Entity e = (Entity){0};
        e.position = (Vector2){820.0f * s, 760.0f * s};
        e.active = true;
        e.isPlayer = false;
        e.isEnemy = false;
        e.isInteractive = true;
        e.npcType = NPC_SIGARACI;
        e.radius = 20.0f;
        e.aiType = AI_NONE;
        e.state = STATE_IDLE;
        if (level->enemyCount < MAX_ENEMIES) {
            level->enemies[level->enemyCount++] = e;
        }
    }

}
