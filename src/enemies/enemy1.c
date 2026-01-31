#include "enemy.h"
#include "../../raylib/src/raymath.h"


#define ENEMY_SHOOT_INTERVAL 3.5f
#define BULLET_SPEED 200.0f
#define BULLET_RADIUS 5.0f
#define BULLET_LIFETIME 2.0f

void UpdateEnemy(Entity *enemy, Vector2 playerPos, Bullet *bulletPool,
                 int maxBullets, float dt, const Level *level) {
  if (!enemy->active)
    return;

  // Detection Logic (Simple Radius)
  if (Vector2Distance(playerPos, enemy->position) < 800.0f) {

    //                                         
    bool hasLineOfSight = true;
    for (int i = 0; i < level->wallCount; i++) {
        Rectangle r = level->walls[i];
        // Check 4 lines of the rect
        Vector2 p1 = {r.x, r.y};
        Vector2 p2 = {r.x + r.width, r.y};
        Vector2 p3 = {r.x + r.width, r.y + r.height};
        Vector2 p4 = {r.x, r.y + r.height};
        
        Vector2 collisionPoint;
        if (CheckCollisionLines(enemy->position, playerPos, p1, p2, &collisionPoint) ||
            CheckCollisionLines(enemy->position, playerPos, p2, p3, &collisionPoint) ||
            CheckCollisionLines(enemy->position, playerPos, p3, p4, &collisionPoint) ||
            CheckCollisionLines(enemy->position, playerPos, p4, p1, &collisionPoint)) {
            hasLineOfSight = false;
            break;
        }
    }
    // Also check closed doors
    for (int i = 0; i < level->doorCount; i++) {
        if (!level->doorsOpen[i]) {
            Rectangle r = level->doors[i];
            Vector2 p1 = {r.x, r.y};
            Vector2 p2 = {r.x + r.width, r.y};
            Vector2 p3 = {r.x + r.width, r.y + r.height};
            Vector2 p4 = {r.x, r.y + r.height};
            
            Vector2 collisionPoint;
            if (CheckCollisionLines(enemy->position, playerPos, p1, p2, &collisionPoint) ||
                CheckCollisionLines(enemy->position, playerPos, p2, p3, &collisionPoint) ||
                CheckCollisionLines(enemy->position, playerPos, p3, p4, &collisionPoint) ||
                CheckCollisionLines(enemy->position, playerPos, p4, p1, &collisionPoint)) {
                hasLineOfSight = false;
                break;
            }
        }
    }

    if (hasLineOfSight) {
        // Timer Logic
        enemy->shootTimer -= dt;
        if (enemy->shootTimer <= 0) {
          enemy->shootTimer = ENEMY_SHOOT_INTERVAL;

          // Shooting Logic: Find dead bullet and fire
          for (int b = 0; b < maxBullets; b++) {
            if (!bulletPool[b].active) {
              bulletPool[b].active = true;
              bulletPool[b].position = enemy->position;
              bulletPool[b].radius = BULLET_RADIUS;
              bulletPool[b].lifeTime = BULLET_LIFETIME;
              bulletPool[b].isPlayerOwned = false; // Enemy Bullet

              Vector2 toPlayer = Vector2Subtract(playerPos, enemy->position);
              bulletPool[b].velocity =
                  Vector2Scale(Vector2Normalize(toPlayer),
                               BULLET_SPEED * 0.6f); // Slower bullets for enemies
              break;
            }
          }
        }
    }
  }
}
