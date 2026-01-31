
#include "../../raylib/src/raylib.h"
#include "../entity.h"
#include "../levels.h"
#include "../types.h"

#ifndef PLAYER_H
#define PLAYER_H

// Initialize player state
Entity InitPlayer(Vector2 spawnPos, Identity startIdentity);

// Update player movement and physics
// Update player movement and physics
void UpdatePlayer(Entity *player, Level *currentLevel, float dt, bool godMode);

#endif // PLAYER_H
