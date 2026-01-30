#ifndef ENEMY_H
#define ENEMY_H

#include "../entity.h"
#include "../levels.h"

// Updates a single enemy instance (AI, Shooting, etc.)
// Returns true if the enemy fired a shot (helper for bullet spawning if needed,
// or handles internal)
void UpdateEnemy(Entity *enemy, Vector2 playerPos, Bullet *bulletPool,
                 int maxBullets, float dt);

#endif // ENEMY_H
