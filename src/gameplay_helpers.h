#ifndef GAMEPLAY_HELPERS_H
#define GAMEPLAY_HELPERS_H

#include "entity.h"
#include "levels.h"
#include "../raylib/src/raymath.h"

// Gameplay helpers used by the main loop.


int Gameplay_GetClosestDoor(const Level *level, Vector2 position);

// Raycasts against walls/doors and returns the hit point (or end if no hit)
Vector2 Gameplay_GetRayHit(Vector2 start, Vector2 end, const Level *level);

// Rotated Rectangle Collision
bool CheckCollisionCircleRotatedRect(Vector2 center, float radius, Rectangle rect, float rotation);

#endif // GAMEPLAY_HELPERS_H
