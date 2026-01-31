#ifndef GAMEPLAY_HELPERS_H
#define GAMEPLAY_HELPERS_H

#include "entity.h"
#include "levels.h"

// Gameplay helpers used by the main loop.

int Gameplay_GetClosestEnemyInRange(const Level *level, Vector2 position, float range);

void Gameplay_HandleEnemyKilled(Level *level,
                               int enemyIndex,
                               Entity *player,
                               Entity *droppedMask,
                               bool *maskActive,
                               float droppedMaskRadius);

#endif // GAMEPLAY_HELPERS_H
