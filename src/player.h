#ifndef PLAYER_H
#define PLAYER_H

#include "../raylib/src/raylib.h"
#include "entity.h"
#include "levels.h"
#include "types.h"


// Initialize player state
Entity InitPlayer(Vector2 spawnPos, Identity startIdentity);

// Update player movement and physics
void UpdatePlayer(Entity *player, Level *currentLevel, float dt);

#endif // PLAYER_H
