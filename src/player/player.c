#include "player.h"
#include "../raylib/src/raymath.h"

Entity InitPlayer(Vector2 spawnPos, Identity startIdentity) {
  Entity player = {.position = spawnPos,
                   .active = true,
                   .identity = startIdentity,
                   .isPlayer = true,
                   .radius = 20.0f,
                   .shootTimer = 0.0f,
                   .magAmmo = 12,      // Default mag size
                   .reserveAmmo = 48,  // Default reserve ammo
                   .isReloading = false,
                   .reloadTimer = 0.0f};
  return player;
}

void UpdatePlayer(Entity *player, Level *currentLevel, float dt) {
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
    Vector2 nextPos = Vector2Add(
        player->position, Vector2Scale(moveInput, player->identity.speed * dt));

    bool blocked = false;

    // Wall Collision
    for (int i = 0; i < currentLevel->wallCount; i++) {
      if (CheckCollisionCircleRec(nextPos, player->radius,
                                  currentLevel->walls[i])) {
        blocked = true;
        break;
      }
    }

    // Door Collision (Physical Lock)
    for (int i = 0; i < currentLevel->doorCount; i++) {
      if (CheckCollisionCircleRec(nextPos, player->radius,
                                  currentLevel->doors[i])) {
        if (player->identity.permissionLevel >= currentLevel->doorPerms[i]) {
          // ALLOW PASS
        } else {
          blocked = true;
        }
      }
    }

    if (!blocked) {
      player->position = nextPos;
    }
    player->velocity = Vector2Scale(moveInput, player->identity.speed);
  }
}
