#include "enemy.h"
#include "../../raylib/src/raymath.h"


#define ENEMY_SHOOT_INTERVAL 2.0f
#define BULLET_SPEED 800.0f
#define BULLET_RADIUS 5.0f
#define BULLET_LIFETIME 2.0f

void UpdateEnemy(Entity *enemy, Vector2 playerPos, Bullet *bulletPool,
                 int maxBullets, float dt) {
  if (!enemy->active)
    return;

  // Detection Logic (Simple Radius)
  if (Vector2Distance(playerPos, enemy->position) < 800.0f) {

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
