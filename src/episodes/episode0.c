#include "../enemies/enemy.h"
#include "../types.h"
#include "episodes.h"

void InitEpisode0(Level *level) {
    if (!level || level->id != 0) {
        return;
    }

    level->wallCount = 0;
    level->doorCount = 0;
    level->enemyCount = 0;
    level->playerSpawn = (Vector2){100.0f, 360.0f};
    level->playerStartId = GetIdentity(ENEMY_CIVILIAN);
    level->winX = -1.0f;

    Identity npcIdentity = {.permissionLevel = PERM_NONE, .color = BLUE, .speed = 0.0f};

    level->enemies[level->enemyCount++] = (Entity){
        .position = {140, 300},
        .active = true,
        .isPlayer = false,
        .radius = 20,
        .identity = npcIdentity,
        .aiType = AI_NONE,
        .state = STATE_IDLE,
        .sightRange = 0.0f,
        .sightAngle = 0.0f,
        .isEnemy = false,
        .isInteractive = true,
        .npcType = NPC_KIZ
    };
    level->enemies[level->enemyCount++] = (Entity){
        .position = {210, 330},
        .active = true,
        .isPlayer = false,
        .radius = 20,
        .identity = npcIdentity,
        .aiType = AI_NONE,
        .state = STATE_IDLE,
        .sightRange = 0.0f,
        .sightAngle = 0.0f,
        .isEnemy = false,
        .isInteractive = true,
        .npcType = NPC_COCUK
    };
    level->enemies[level->enemyCount++] = (Entity){
        .position = {740, 430},
        .active = true,
        .isPlayer = false,
        .radius = 20,
        .identity = npcIdentity,
        .aiType = AI_NONE,
        .state = STATE_IDLE,
        .sightRange = 0.0f,
        .sightAngle = 0.0f,
        .isEnemy = false,
        .isInteractive = true,
        .npcType = NPC_BALIKCI
    };
    level->enemies[level->enemyCount++] = (Entity){
        .position = {820, 760},
        .active = true,
        .isPlayer = false,
        .radius = 20,
        .identity = npcIdentity,
        .aiType = AI_NONE,
        .state = STATE_IDLE,
        .sightRange = 0.0f,
        .sightAngle = 0.0f,
        .isEnemy = false,
        .isInteractive = true,
        .npcType = NPC_SIGARACI
    };
}
