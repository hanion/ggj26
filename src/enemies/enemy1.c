#include "enemy.h"
#include "../../raylib/src/raymath.h"
#include "../gameplay_helpers.h"


#define ENEMY_SHOOT_INTERVAL 2.0f
#define BULLET_SPEED 800.0f
#define BULLET_RADIUS 5.0f
#define BULLET_LIFETIME 2.0f

// Helper: Check if line segment (p1-p2) intersects a rectangle
static bool CheckCollisionSegmentRec(Vector2 p1, Vector2 p2, Rectangle rec) {
    // 1. Box check (optimization)
    float minX = fminf(p1.x, p2.x);
    float maxX = fmaxf(p1.x, p2.x);
    float minY = fminf(p1.y, p2.y);
    float maxY = fmaxf(p1.y, p2.y);
    
    if (maxX < rec.x || minX > rec.x + rec.width ||
        maxY < rec.y || minY > rec.y + rec.height) {
        // Broad phase miss
        return false;
    }

    // 2. Check each edge of the rectangle
    Vector2 bl = { rec.x, rec.y + rec.height };
    Vector2 br = { rec.x + rec.width, rec.y + rec.height };
    Vector2 tr = { rec.x + rec.width, rec.y };
    Vector2 tl = { rec.x, rec.y };
    
    Vector2 collisionPoint;
    if (CheckCollisionLines(p1, p2, tl, bl, &collisionPoint)) return true; // Left
    if (CheckCollisionLines(p1, p2, bl, br, &collisionPoint)) return true; // Bottom
    if (CheckCollisionLines(p1, p2, br, tr, &collisionPoint)) return true; // Right
    if (CheckCollisionLines(p1, p2, tr, tl, &collisionPoint)) return true; // Top
    
    // 3. Check if line is completely INSIDE rectangle
    // If p1 is inside, then it intersects the area (conceptually invalid LoS)
    if (p1.x > rec.x && p1.x < rec.x+rec.width && p1.y > rec.y && p1.y < rec.y+rec.height) return true;
    
    return false;
}

// Check if enemy can see target (distance, angle, walls)
bool CheckLineOfSight(Entity *enemy, Vector2 target, Level *level) {
    if (!enemy->active) return false;

    // 1. Distance Check
    float dist = Vector2Distance(enemy->position, target);
    if (dist > enemy->sightRange) return false;

    // 2. Angle Check (Cone)
    Vector2 toTarget = Vector2Subtract(target, enemy->position);
    float angleToTarget = atan2f(toTarget.y, toTarget.x) * RAD2DEG;
    float angleDiff = fabsf(angleToTarget - enemy->rotation);
    // Wrap angle diff
    while (angleDiff > 180) angleDiff -= 360;
    while (angleDiff < -180) angleDiff += 360;
    if (fabsf(angleDiff) > enemy->sightAngle / 2.0f) return false;

    // 3. Wall Check (Segment Intersection)
    for (int i = 0; i < level->wallCount; i++) {
        if (CheckCollisionSegmentRec(enemy->position, target, level->walls[i])) {
            return false;
        }
    }
    
    // Check Doors (Closed ones check)
    for (int i = 0; i < level->doorCount; i++) {
        // If door is closed, it blocks sight
        if (!level->doors[i].isOpen) {
            if (CheckCollisionSegmentRec(enemy->position, target, level->doors[i].rect)) {
                return false;
            }
        }
    }

    return true;
}

// Helper for collision
static bool MoveEnemyWithCollision(Entity *enemy, Vector2 delta, const Level *level) {
    Vector2 originalPos = enemy->position;
    bool hit = false;

    // --- X AXIS ---
    enemy->position.x += delta.x;
    bool blocked_x = false;

    for (int i = 0; i < level->wallCount; i++) {
        if (CheckCollisionCircleRec(enemy->position, enemy->radius, level->walls[i])) {
            blocked_x = true;
            break;
        }
    }
    for (int i = 0; i < level->doorCount && !blocked_x; i++) {
        if (CheckCollisionCircleRec(enemy->position, enemy->radius, level->doors[i].rect)) {
             if (enemy->identity.permissionLevel < level->doors[i].requiredPerm && !level->doors[i].isOpen) {
                blocked_x = true;
            }
        }
    }

    if (blocked_x) {
        enemy->position.x = originalPos.x;
        hit = true;
    }

    // --- Y AXIS ---
    enemy->position.y += delta.y;
    bool blocked_y = false;

    for (int i = 0; i < level->wallCount; i++) {
        if (CheckCollisionCircleRec(enemy->position, enemy->radius, level->walls[i])) {
            blocked_y = true;
            break;
        }
    }
    for (int i = 0; i < level->doorCount && !blocked_y; i++) {
        if (CheckCollisionCircleRec(enemy->position, enemy->radius, level->doors[i].rect)) {
             if (enemy->identity.permissionLevel < level->doors[i].requiredPerm && !level->doors[i].isOpen) {
                blocked_y = true;
            }
        }
    }

    if (blocked_y) {
        enemy->position.y = originalPos.y;
        hit = true;
    }
    
    return hit;
}

void UpdateEnemy(Entity *enemy, Vector2 playerPos, Level *level, Bullet *bulletPool,
                 int maxBullets, float dt) {
  if (!enemy->active) return;
  if (enemy->state == STATE_BEING_CHOKED) return; // Frozen while being choked
  
  bool seesPlayer = CheckLineOfSight(enemy, playerPos, level);
  
  // -- AI STATE MACHINE TRANSITIONS --
  if (seesPlayer) {
      if (enemy->state != STATE_ATTACK) {
          // Triggered!
          enemy->state = STATE_ATTACK;
      }
      enemy->lastKnownPlayerPos = playerPos;
      enemy->searchTimer = 0.1f; // Reset search timer
  } else {
      if (enemy->state == STATE_ATTACK) {
          // Lost sight
          enemy->state = STATE_SEARCH; // Both types search now
          enemy->searchTimer = enemy->IGotHitImSearchingThePlayerForHowManySeconds; // Use configured time
          // Or should we use a shorter time if just lost sight vs hit? 
          // User said "If cant see player even if in that position it will search for a time called IGotHit..."
          // This applies to the HIT case.
          // For natural lost sight, maybe keep it similar or shorter?
          // Let's use the variable to be consistent with "Search" property.
      }
  }

  // -- AI BEHAVIOR --
  switch (enemy->state) {
      case STATE_IDLE:
          if (enemy->aiType == AI_WALKER) {
              // Idle for a bit, then pick a new patrol point
              enemy->searchTimer -= dt; // Reusing searchTimer for idle/patrol wait
              if (enemy->searchTimer <= 0) {
                  // Pick a random point near patrolStart
                  float angle = (float)GetRandomValue(0, 360) * DEG2RAD;
                  float dist = (float)GetRandomValue(50, 200);
                  Vector2 offset = { cosf(angle)*dist, sinf(angle)*dist };
                  // Ensure random point is valid? For now just try to go there
                  enemy->lastKnownPlayerPos = Vector2Add(enemy->position, offset); 
                  enemy->state = STATE_PATROL;
              }
          } else if (enemy->aiType == AI_GUARDIAN) {
              // GUARDIAN BEHAVIOR: Look at closest door
              int doorIdx = Gameplay_GetClosestDoor(level, enemy->position);
              if (doorIdx != -1) {
                  Vector2 doorCenter = {
                      level->doors[doorIdx].rect.x + level->doors[doorIdx].rect.width/2,
                      level->doors[doorIdx].rect.y + level->doors[doorIdx].rect.height/2
                  };
                  Vector2 toDoor = Vector2Subtract(doorCenter, enemy->position);
                  float targetAngle = atan2f(toDoor.y, toDoor.x) * RAD2DEG;
                  
                  // Add subtle random movement (idle jitter)
                  // Change target slightly every second or so?
                  // We can use searchTimer as a "jitter timer"
                  enemy->searchTimer -= dt;
                  if (enemy->searchTimer <= 0) {
                       enemy->searchTimer = (float)GetRandomValue(5, 15) / 10.0f; // 0.5s - 1.5s
                       // This is a bit hacky, storing offset in 'lastKnownPlayerPos.x' just for temp storage?
                       // Or just add immediate noise.
                       enemy->lastKnownPlayerPos.x = (float)GetRandomValue(-20, 20); 
                  }
                  
                  targetAngle += enemy->lastKnownPlayerPos.x; // Add the noise

                  // Smooth rotation
                  float angleDiff = targetAngle - enemy->rotation;
                  while (angleDiff > 180) angleDiff -= 360;
                  while (angleDiff < -180) angleDiff += 360;
                  enemy->rotation += angleDiff * 2.0f * dt; // Slow turn
              }
          }
          break;
          
      case STATE_ATTACK:
          // Aim at player
          {
              Vector2 toPlayer = Vector2Subtract(playerPos, enemy->position);
              
              float targetAngle = atan2f(toPlayer.y, toPlayer.x) * RAD2DEG;
              
              // Smooth rotation
              float angleDiff = targetAngle - enemy->rotation;
                while (angleDiff > 180) angleDiff -= 360;
                while (angleDiff < -180) angleDiff += 360;
              enemy->rotation += angleDiff * 70.0f * dt;

              
              if (enemy->aiType == AI_WALKER) {
                  // Chase Player
                  float dist = Vector2Length(toPlayer);
                  if (dist > 100) { // Keep some distance
                       Vector2 moveDir = Vector2Normalize(toPlayer);
                       Vector2 delta = Vector2Scale(moveDir, enemy->identity.speed * dt);
                       MoveEnemyWithCollision(enemy, delta, level);
                  }
              }
              
              // Shoot
              enemy->shootTimer -= dt;
              if (enemy->shootTimer <= 0 && seesPlayer) { // Only shoot if we definitely see player
                  enemy->shootTimer = ENEMY_SHOOT_INTERVAL;
                   for (int b = 0; b < maxBullets; b++) {
                        if (!bulletPool[b].active) {
                            bulletPool[b].active = true;
                            bulletPool[b].position = enemy->position;
                            bulletPool[b].radius = BULLET_RADIUS;
                            bulletPool[b].lifeTime = BULLET_LIFETIME;
                            bulletPool[b].isPlayerOwned = false; 

                            Vector2 fireDir = Vector2Subtract(playerPos, enemy->position);
                            // Add inaccuracy
                            fireDir = Vector2Rotate(fireDir, (float)GetRandomValue(-5, 5) * DEG2RAD);
                            
                            bulletPool[b].velocity = Vector2Scale(Vector2Normalize(fireDir), BULLET_SPEED * 0.6f);
                            break;
                        }
                   }
              }
          }
          break;
          
      case STATE_SEARCH:
          if (enemy->aiType == AI_WALKER) {
               // Walker: Move to last known position
               Vector2 toLast = Vector2Subtract(enemy->lastKnownPlayerPos, enemy->position);
               float dist = Vector2Length(toLast);
               
               if (dist > 20) {
                   // Moving to search target
                   Vector2 delta = Vector2Scale(Vector2Normalize(toLast), enemy->identity.speed * dt);
                   bool bumped = MoveEnemyWithCollision(enemy, delta, level); 
                   
                   if (bumped) {
                        dist = 0; 
                   } else {
                        float targetAngle = atan2f(toLast.y, toLast.x) * RAD2DEG;
                        float angleDiff = targetAngle - enemy->rotation;
                        while (angleDiff > 180) angleDiff -= 360;
                        while (angleDiff < -180) angleDiff += 360;
                        enemy->rotation += angleDiff * 15.0f * dt; 
                   }
               }
               
               if (dist <= 20) {
                   // Arrived, look around
                   enemy->searchTimer -= dt;
                   enemy->rotation += 180 * dt; 
                   
                   if (enemy->searchTimer <= 0) {
                       enemy->state = STATE_PATROL;
                       enemy->searchTimer = 0.5f; 
                   }
               }
          } else if (enemy->aiType == AI_GUARDIAN) {
               // Guardian: Stationary Search
               // Just turn towards the threat and wait
               Vector2 toLast = Vector2Subtract(enemy->lastKnownPlayerPos, enemy->position);
               float targetAngle = atan2f(toLast.y, toLast.x) * RAD2DEG;
               float angleDiff = targetAngle - enemy->rotation;
               while (angleDiff > 180) angleDiff -= 360;
               while (angleDiff < -180) angleDiff += 360;
               enemy->rotation += angleDiff * 15.0f * dt; // Fast turn to look

               enemy->searchTimer -= dt;
               if (enemy->searchTimer <= 0) {
                   enemy->state = STATE_IDLE; // Return to post
               }
          }
          break;
          
      case STATE_PATROL:
          // Move to random patrol point (stored in lastKnownPlayerPos)
          {
              Vector2 toTarget = Vector2Subtract(enemy->lastKnownPlayerPos, enemy->position);
              float dist = Vector2Length(toTarget);
              
              if (dist > 10) {
                  Vector2 delta = Vector2Scale(Vector2Normalize(toTarget), enemy->identity.speed * 0.5f * dt);
                  bool bumped = MoveEnemyWithCollision(enemy, delta, level);
                  
                  if (bumped) {
                      // We hit a wall while blindly patrolling. This target is bad. Find new one.
                      enemy->state = STATE_IDLE;
                      enemy->searchTimer = 0.5f; // Wait briefly
                  } else {
                      float targetAngle = atan2f(toTarget.y, toTarget.x) * RAD2DEG;
                      // Smooth rotation
                      float angleDiff = targetAngle - enemy->rotation;
                      while (angleDiff > 180) angleDiff -= 360;
                      while (angleDiff < -180) angleDiff += 360;
                      enemy->rotation += angleDiff * 3.0f * dt; // Slower turn for patrol
                  }
              } else {
                  // Reached patrol point
                  enemy->state = STATE_IDLE; 
                  enemy->searchTimer = (float)GetRandomValue(10, 30) / 10.0f; // Idle for 1-3 seconds
              }
          }
          break;
  }
}
