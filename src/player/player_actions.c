#include "player_actions.h"

#include "../raylib/src/raymath.h"

int PlayerActions_GetClosestEnemyInRange(const Level *level, Vector2 position, float range) {
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

void PlayerActions_HandleEnemyKilled(Level *level,
                                    int enemyIndex,
                                    Entity *player,
                                    Entity *droppedMasks,
                                    int maxMasks,
                                    float droppedMaskRadius) {
    if (!level || !player || !droppedMasks) return;
    if (enemyIndex < 0 || enemyIndex >= level->enemyCount) {
        return;
    }
    if (!level->enemies[enemyIndex].active) {
        return;
    }

    level->enemies[enemyIndex].active = false;

    // Find first inactive mask slot
    for (int i = 0; i < maxMasks; i++) {
        if (!droppedMasks[i].active) {
            droppedMasks[i].identity = level->enemies[enemyIndex].identity;
            droppedMasks[i].position = level->enemies[enemyIndex].position;
            droppedMasks[i].radius = droppedMaskRadius;
            droppedMasks[i].active = true;
            break;
        }
    }

    // Auto-equip handgun after a kill (so player can shoot instead of staying on knife).
    // If you later add actual weapon pickups, move this to that pickup logic instead.
    if (player->equipmentState == PLAYER_EQUIP_KNIFE || player->equipmentState == PLAYER_EQUIP_BARE_HANDS) {
        player->equipmentState = PLAYER_EQUIP_HANDGUN;
        TraceLog(LOG_INFO, "Player auto-equipped handgun after kill");
    }
}
