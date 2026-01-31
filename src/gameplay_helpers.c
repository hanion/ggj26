#include "gameplay_helpers.h"

#include "../raylib/src/raylib.h"
#include "../raylib/src/raymath.h"

int Gameplay_GetClosestEnemyInRange(const Level *level, Vector2 position, float range) {
    if (!level) return -1;

    float closestDist = range;
    int closestIndex = -1;
    for (int i = 0; i < level->enemyCount; i++) {
        if (!level->enemies[i].active) {
            continue;
        }
        float dist = Vector2Distance(position, level->enemies[i].position);
        if (dist <= closestDist) {
            closestDist = dist;
            closestIndex = i;
        }
    }
    return closestIndex;
}

void Gameplay_HandleEnemyKilled(Level *level,
                               int enemyIndex,
                               Entity *player,
                               Entity *droppedMask,
                               bool *maskWasActivated,
                               float droppedMaskRadius) {
    if (!level || !player || !droppedMask || !maskWasActivated) return;
    if (enemyIndex < 0 || enemyIndex >= level->enemyCount) {
        return;
    }
    if (!level->enemies[enemyIndex].active) {
        return;
    }

    level->enemies[enemyIndex].active = false;

    *maskWasActivated = true;
    droppedMask->identity = level->enemies[enemyIndex].identity;
    droppedMask->position = level->enemies[enemyIndex].position;
    droppedMask->radius = droppedMaskRadius;
    droppedMask->active = true;

    // Auto-equip handgun after a kill (so player can shoot instead of staying on knife).
    // If you later add actual weapon pickups, move this to that pickup logic instead.
    if (player->equipmentState == PLAYER_EQUIP_KNIFE || player->equipmentState == PLAYER_EQUIP_BARE_HANDS) {
        player->equipmentState = PLAYER_EQUIP_HANDGUN;
        TraceLog(LOG_INFO, "Player auto-equipped handgun after kill");
    }
}


