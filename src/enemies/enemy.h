#ifndef ENEMY_H
#define ENEMY_H

#include "../entity.h"
#include "../levels.h"

// Factory to create a new enemy based on type
Entity InitEnemy(Vector2 position, EnemyType type);

// Get default identity for a type (useful for player init or other needs)
Identity GetIdentity(EnemyType type);

// Check if enemy can see target (distance, angle, walls)
bool CheckLineOfSight(Entity *enemy, Vector2 target, Level *level);

// Updates a single enemy instance (AI, Shooting, etc.)
// Returns true if the enemy fired a shot (helper for bullet spawning if needed,
// or handles internal)
void UpdateEnemy(Entity *enemy, Vector2 playerPos, Level *level, Bullet *bulletPool,
                 int maxBullets, float dt);

#endif // ENEMY_H
