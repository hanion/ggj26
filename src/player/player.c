#include "player.h"
#include "../../raylib/src/raymath.h"
#include "../masks/mask.h"

Entity InitPlayer(Vector2 spawnPos, Identity startIdentity) {
  Entity player = {0};
  player.position = spawnPos;
  player.active = true;
  player.identity = startIdentity;
  player.isPlayer = true;
  player.radius = 20.0f;
  player.shootTimer = 0.0f;
  player.shootTimer = 0.0f;
  
  // STATS
  player.health = 100.0f;
  player.maxHealth = 100.0f;
  player.speedMultiplier = 1.0f;
  player.isInvisible = false;
  
  // INIT INVENTORY
  // All 3 Slots start with Knife
  for(int i=0; i<3; i++) {
       player.inventory.gunSlots[i].type = GUN_KNIFE;
       player.inventory.gunSlots[i].active = true;
       player.inventory.gunSlots[i].damage = 50.0f;
       player.inventory.gunSlots[i].range = 100.0f;
       player.inventory.gunSlots[i].cooldown = 0.5f;
  }
  player.inventory.currentGunIndex = 0;

  // Empty masks
  for(int i=0; i<3; i++) player.inventory.maskSlots[i].type = MASK_NONE;
  player.inventory.currentMaskIndex = 0;

  // Card
  player.inventory.card.level = PERM_NONE;
  player.inventory.card.collected = false;

  return player;
}

void UpdatePlayer(Entity *player, Level *currentLevel, float dt, bool godMode) {
  // Update Masks
  Masks_Update(player, dt);

  Vector2 moveInput = {0};
  if (IsKeyDown(KEY_W))
    moveInput.y -= 1.0f;
  if (IsKeyDown(KEY_S))
    moveInput.y += 1.0f;
  if (IsKeyDown(KEY_A))
    moveInput.x -= 1.0f;
  if (IsKeyDown(KEY_D))
    moveInput.x += 1.0f;

  player->velocity = (Vector2){0};

  if (Vector2Length(moveInput) > 0) {
    moveInput = Vector2Normalize(moveInput);

    Vector2 delta = Vector2Scale(moveInput, player->identity.speed * player->speedMultiplier * dt);
    Vector2 newPos = player->position;

    // --- X AXIS ---
    Vector2 xPos = newPos;
    xPos.x += delta.x;

    bool blocked_x = false;
    for (int i = 0; i < currentLevel->wallCount; i++) {
      if (CheckCollisionCircleRec(xPos, player->radius,
                    currentLevel->walls[i])) {
        blocked_x = true;
        break;
      }
    }

    if (!godMode) {
        PermissionLevel myLevel = player->inventory.card.level;
        for (int i = 0; i < currentLevel->doorCount && !blocked_x; i++) {
          if (CheckCollisionCircleRec(xPos, player->radius, currentLevel->doors[i])) {
            if (myLevel < currentLevel->doorPerms[i]) {
              blocked_x = true;
            }
          }
        }
    }

    if (!blocked_x) {
      newPos.x = xPos.x;
    }

    // Check Enemy Collision (X)
    for (int i = 0; i < currentLevel->enemyCount; i++) {
        if (!currentLevel->enemies[i].active) continue;
        if (CheckCollisionCircles(newPos, player->radius, currentLevel->enemies[i].position, currentLevel->enemies[i].radius)) {
            // Revert X change if colliding with enemy
            newPos.x = player->position.x; 
            break;
        }
    }

    // --- Y AXIS ---
    Vector2 yPos = newPos;
    yPos.y += delta.y;

    bool blocked_y = false;
    for (int i = 0; i < currentLevel->wallCount; i++) {
      if (CheckCollisionCircleRec(yPos, player->radius, currentLevel->walls[i])) {
        blocked_y = true;
        break;
      }
    }

    if (!godMode) {
        PermissionLevel myLevel = player->inventory.card.level;
        for (int i = 0; i < currentLevel->doorCount && !blocked_y; i++) {
          if (CheckCollisionCircleRec(yPos, player->radius, currentLevel->doors[i])) {
            if (myLevel < currentLevel->doorPerms[i]) {
              blocked_y = true;
            }
          }
        }
    }

    if (!blocked_y) {
      newPos.y = yPos.y;
    }

    // Check Enemy Collision (Y)
    for (int i = 0; i < currentLevel->enemyCount; i++) {
        if (!currentLevel->enemies[i].active) continue;
        if (CheckCollisionCircles(newPos, player->radius, currentLevel->enemies[i].position, currentLevel->enemies[i].radius)) {
            // Revert Y change if colliding with enemy
            newPos.y = player->position.y; // Note: player->position.y is the original Y (since we haven't updated player->position yet)
            // Wait, newPos.x might have changed from the X step. We only want to revert Y.
            // If we revert to player->position.y, that is correct (Keep X change, revert Y change).
            break;
        }
    }

    player->position = newPos;
    player->velocity = Vector2Scale(moveInput, player->identity.speed);
  }
}
