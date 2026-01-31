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

    Vector2 delta = Vector2Scale(moveInput, player->identity.speed * dt);
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

    for (int i = 0; i < currentLevel->doorCount && !blocked_x; i++) {
      if (CheckCollisionCircleRec(xPos, player->radius, currentLevel->doors[i])) {
        if (player->identity.permissionLevel < currentLevel->doorPerms[i]) {
          blocked_x = true;
        }
      }
    }

    if (!blocked_x) {
      newPos.x = xPos.x;
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

    for (int i = 0; i < currentLevel->doorCount && !blocked_y; i++) {
      if (CheckCollisionCircleRec(yPos, player->radius, currentLevel->doors[i])) {
        if (player->identity.permissionLevel < currentLevel->doorPerms[i]) {
          blocked_y = true;
        }
      }
    }

    if (!blocked_y) {
      newPos.y = yPos.y;
    }

    player->position = newPos;
    player->velocity = Vector2Scale(moveInput, player->identity.speed);
  }
}
