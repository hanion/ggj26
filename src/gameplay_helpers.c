#include "gameplay_helpers.h"

#include "../raylib/src/raylib.h"
#include "../raylib/src/raymath.h"

int Gameplay_GetClosestEnemyInRange(const Level *level, Vector2 position, float range) {
    if (!level) return -1;

    float closestDist = range;
    int closestIndex = -1;
    for (int i = 0; i < level->enemyCount; i++) {
        if (!level->enemies[i].active || !level->enemies[i].isEnemy) {
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

    // Auto-equip handgun after a kill if we are using knife/bare hands
    GunType currentType = player->inventory.gunSlots[player->inventory.currentGunIndex].type;
    if (currentType == GUN_KNIFE || currentType == GUN_NONE) {
        // Find a handgun or better
        for (int i=0; i<3; i++) {
             if (player->inventory.gunSlots[i].type == GUN_HANDGUN) {
                  player->inventory.currentGunIndex = i;
                  TraceLog(LOG_INFO, "Player auto-switched to Handgun slot %d", i);
                  break;
             }
        }
    }
}

int Gameplay_GetClosestDoor(const Level *level, Vector2 position) {
    if (!level || level->doorCount <= 0) return -1;

    int closestIdx = -1;
    float closestDistSqr = 99999999.0f;

    for (int i = 0; i < level->doorCount; i++) {
        Vector2 center = { 
            level->doors[i].x + level->doors[i].width/2,
            level->doors[i].y + level->doors[i].height/2
        };
        float d2 = Vector2DistanceSqr(position, center);
        if (d2 < closestDistSqr) {
            closestDistSqr = d2;
            closestIdx = i;
        }
    }
    return closestIdx;
}

