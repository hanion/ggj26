#ifndef GAMEPLAY_HELPERS_H
#define GAMEPLAY_HELPERS_H

#include "entity.h"
#include "levels.h"
#include "../raylib/src/raymath.h"

// Gameplay helpers used by the main loop.

int Gameplay_GetClosestEnemyInRange(const Level *level, Vector2 position, float range);

void Gameplay_HandleEnemyKilled(Level *level,
                               int enemyIndex,
                               Entity *player,
                               Entity *droppedMask,
                               bool *maskActive,
                               float droppedMaskRadius);

// Raycasts against walls/doors and returns the hit point (or end if no hit)
// Raycasts against walls/doors and returns the hit point (or end if no hit)
static inline Vector2 Gameplay_GetRayHit(Vector2 start, Vector2 end, const Level *level) {
    if (!level) return end;

    Vector2 closestHit = end;
    float closestDistSqr = Vector2DistanceSqr(start, end);

    // Flattened loop to check intersections
    for (int i = 0; i < level->wallCount + level->doorCount; i++) {
        Rectangle rec;
        if (i < level->wallCount) {
            rec = level->walls[i];
        } else {
            int doorIdx = i - level->wallCount;
            if (level->doorsOpen[doorIdx]) continue;
            rec = level->doors[doorIdx];
        }

       Vector2 tl = {rec.x, rec.y};
       Vector2 tr = {rec.x + rec.width, rec.y};
       Vector2 bl = {rec.x, rec.y + rec.height};
       Vector2 br = {rec.x + rec.width, rec.y + rec.height};

       Vector2 hit;
       if (CheckCollisionLines(start, end, tl, tr, &hit)) {
           float d2 = Vector2DistanceSqr(start, hit);
           if (d2 < closestDistSqr) { closestDistSqr = d2; closestHit = hit; }
       }
       if (CheckCollisionLines(start, end, tr, br, &hit)) {
           float d2 = Vector2DistanceSqr(start, hit);
           if (d2 < closestDistSqr) { closestDistSqr = d2; closestHit = hit; }
       }
       if (CheckCollisionLines(start, end, br, bl, &hit)) {
           float d2 = Vector2DistanceSqr(start, hit);
           if (d2 < closestDistSqr) { closestDistSqr = d2; closestHit = hit; }
       }
       if (CheckCollisionLines(start, end, bl, tl, &hit)) {
           float d2 = Vector2DistanceSqr(start, hit);
           if (d2 < closestDistSqr) { closestDistSqr = d2; closestHit = hit; }
       }
    }

    return closestHit;
}

#endif // GAMEPLAY_HELPERS_H
