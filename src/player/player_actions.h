#ifndef PLAYER_ACTIONS_H
#define PLAYER_ACTIONS_H

#include <stdbool.h>

#include "../raylib/src/raylib.h"

#include "../entity.h"
#include "../levels.h"

// Player-driven gameplay actions (combat/interaction helpers).

int PlayerActions_GetClosestEnemyInRange(const Level *level, Vector2 position, float range);

void PlayerActions_HandleEnemyKilled(Level *level,
                                    int enemyIndex,
                                    Entity *player,
                                    Entity *droppedMask,
                                    bool *maskWasActivated,
                                    float droppedMaskRadius);

#endif // PLAYER_ACTIONS_H
