#include <math.h>
#include <stdio.h>

#include "../raylib/src/raylib.h"
#include "../raylib/src/raymath.h"
#include "entity.h"
#include "types.h"


#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720

#define MAX_BULLETS 100
#define BULLET_SPEED 800.0f
#define BULLET_RADIUS 5.0f
#define BULLET_LIFETIME 2.0f // Seconds

#define ENEMY_SHOOT_INTERVAL 2.0f // Seconds

// Quick helpers for bitwise checks
bool HasAbility(Identity id, AbilityFlags flag) {
  return (id.abilities & flag) != 0;
}

int main(void) {
  InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Identity Theft - Episode 1");
  SetTargetFPS(60);

  // --- GAME STATE INITIALIZATION ---
  bool gameOver = false;
  bool gameWon = false;

  // Identities
  Identity idCivilian = {.permissionLevel = PERM_CIVILIAN,
                         .abilities = ABILITY_SHOOT,
                         .color = BLUE,
                         .speed = 220.0f};
  Identity idStaff = {.permissionLevel = PERM_STAFF,
                      .abilities = ABILITY_SHOOT,
                      .color = GREEN,
                      .speed = 210.0f};
  Identity idGuard = {.permissionLevel = PERM_GUARD,
                      .abilities = ABILITY_SHOOT | ABILITY_PUNCH,
                      .color = RED,
                      .speed = 200.0f};
  Identity idAdmin = {.permissionLevel = PERM_ADMIN,
                      .abilities = ABILITY_SHOOT | ABILITY_PUNCH | ABILITY_DASH,
                      .color = PURPLE,
                      .speed = 250.0f};

  // Player
  Entity player = {
      .position = {100.0f, 360.0f}, // Start near left edge, centered vertically
      .active = true,
      .identity = idCivilian,
      .isPlayer = true,
      .radius = 20.0f};

  // Camera
  Camera2D camera = {0};
  camera.target = player.position;
  camera.offset = (Vector2){SCREEN_WIDTH / 2.0f, SCREEN_HEIGHT / 2.0f};
  camera.rotation = 0.0f;
  camera.zoom = 1.0f;

  // --- LEVEL DESIGN: EPISODE 1 ---
  // A long corridor progressing rightwards.
  // X=0 to 1000: Area 1 (Staff Needed)
  // X=1000 to 2000: Area 2 (Guard Needed)
  // X=2000 to 3000: Area 3 (Admin Needed) -> WIN
  // Y=0 to 720: playable height

  // Walls (Simple borders + Dividers)
  Rectangle walls[] = {
      {-50, -50, 4100, 50}, // Top Border (from start to end)
      {-50, 720, 4100, 50}, // Bottom Border
      {-50, 0, 50, 720},    // Start Cap
      {4000, 0, 50, 720},   // End Cap
      // Room Dividers (with gap for door)
      {1000, 0, 50, 300},   // Divider 1 Top
      {1000, 420, 50, 300}, // Divider 1 Bottom
      {2000, 0, 50, 300},   // Divider 2 Top
      {2000, 420, 50, 300}, // Divider 2 Bottom
      {3000, 0, 50, 300},   // Divider 3 Top
      {3000, 420, 50, 300}  // Divider 3 Bottom
  };
  int wallCount = sizeof(walls) / sizeof(walls[0]);

  // Doors
  // Located in the gaps of dividers
  Rectangle doors[] = {
      {1000, 300, 50, 120}, // Door 1: Requires Level 1 (Staff)
      {2000, 300, 50, 120}, // Door 2: Requires Level 2 (Guard)
      {3000, 300, 50, 120}  // Door 3: Requires Level 3 (Admin)
  };
  PermissionLevel doorPerms[] = {PERM_STAFF, PERM_GUARD, PERM_ADMIN};
  bool doorsOpen[] = {false, false, false};
  int doorCount = 3;

  // Enemies
  // One per section guarding the way
  Entity enemies[] = {{// Enemy 1 (Staff)
                       .position = {800.0f, 360.0f},
                       .active = true,
                       .isPlayer = false,
                       .radius = 20.0f,
                       .identity = idStaff,
                       .shootTimer = ENEMY_SHOOT_INTERVAL},
                      {// Enemy 2 (Guard)
                       .position = {1800.0f, 360.0f},
                       .active = true,
                       .isPlayer = false,
                       .radius = 20.0f,
                       .identity = idGuard,
                       .shootTimer = ENEMY_SHOOT_INTERVAL},
                      {// Enemy 3 (Admin)
                       .position = {2800.0f, 360.0f},
                       .active = true,
                       .isPlayer = false,
                       .radius = 20.0f,
                       .identity = idAdmin,
                       .shootTimer = ENEMY_SHOOT_INTERVAL}};
  int enemyCount = 3;

  // Bullets
  Bullet bullets[MAX_BULLETS] = {0};

  // Drops
  Entity droppedMask = {0}; // Single drop slot implies you pick it up before
                            // killing next, or overrides.
  bool maskActive = false;

  while (!WindowShouldClose()) {
    float dt = GetFrameTime();

    // Game Over / Win Logic Inputs
    if (gameOver || gameWon) {
      BeginDrawing();
      ClearBackground(BLACK);
      if (gameOver) {
        DrawText("GAME OVER", SCREEN_WIDTH / 2 - 100, SCREEN_HEIGHT / 2 - 20,
                 40, RED);
        DrawText("Press R to Restart", SCREEN_WIDTH / 2 - 100,
                 SCREEN_HEIGHT / 2 + 30, 20, RAYWHITE);
      } else {
        DrawText("EPISODE 1 COMPLETE!", SCREEN_WIDTH / 2 - 150,
                 SCREEN_HEIGHT / 2 - 20, 40, GOLD);
        DrawText("You have escaped.", SCREEN_WIDTH / 2 - 100,
                 SCREEN_HEIGHT / 2 + 30, 20, RAYWHITE);
      }
      EndDrawing();

      if (IsKeyPressed(KEY_R)) {
        // RESET LEVEL (Crude reload)
        gameOver = false;
        gameWon = false;
        player.position = (Vector2){100.0f, 360.0f};
        player.identity = idCivilian;
        // Reset Enemies
        enemies[0].active = true;
        enemies[0].shootTimer = ENEMY_SHOOT_INTERVAL;
        enemies[1].active = true;
        enemies[1].shootTimer = ENEMY_SHOOT_INTERVAL;
        enemies[2].active = true;
        enemies[2].shootTimer = ENEMY_SHOOT_INTERVAL;
        // Reset Doors
        doorsOpen[0] = false;
        doorsOpen[1] = false;
        doorsOpen[2] = false;
        // Clear bullets
        for (int i = 0; i < MAX_BULLETS; i++)
          bullets[i].active = false;
        maskActive = false;
      }
      continue;
    }

    // --- UPDATE ---

    // Player Movement
    Vector2 moveInput = {0};
    if (IsKeyDown(KEY_W))
      moveInput.y -= 1.0f;
    if (IsKeyDown(KEY_S))
      moveInput.y += 1.0f;
    if (IsKeyDown(KEY_A))
      moveInput.x -= 1.0f;
    if (IsKeyDown(KEY_D))
      moveInput.x += 1.0f;

    if (Vector2Length(moveInput) > 0) {
      moveInput = Vector2Normalize(moveInput);
      Vector2 nextPos = Vector2Add(
          player.position, Vector2Scale(moveInput, player.identity.speed * dt));

      bool blocked = false;

      // Wall Collision
      for (int i = 0; i < wallCount; i++) {
        if (CheckCollisionCircleRec(nextPos, player.radius, walls[i])) {
          blocked = true;
          break;
        }
      }

      // Door Collision (Physical Lock)
      for (int i = 0; i < doorCount; i++) {
        // Check if approaching door i
        if (CheckCollisionCircleRec(nextPos, player.radius, doors[i])) {
          // Check access
          if (player.identity.permissionLevel >= doorPerms[i]) {
            // ALLOW PASS -> Keep blocked false
          } else {
            // DENY PASS
            blocked = true;
          }
        }
      }

      if (!blocked) {
        player.position = nextPos;
      }
    }

    // Camera Update
    camera.target = Vector2Lerp(camera.target, player.position, 0.1f);
    // Clamp cameraY to center so we only scroll X? Or follow both. Let's follow
    // both but clamps might be nice. For this corridor, vertical clamp is good
    // visual polish but simplest is just follow.

    // Player Aiming
    Vector2 mouseWorld = GetScreenToWorld2D(GetMousePosition(), camera);
    Vector2 aimDir = Vector2Subtract(mouseWorld, player.position);
    player.rotation = atan2f(aimDir.y, aimDir.x) * RAD2DEG;

    // Player Shooting
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
      if (HasAbility(player.identity, ABILITY_SHOOT)) {
        for (int i = 0; i < MAX_BULLETS; i++) {
          if (!bullets[i].active) {
            bullets[i].active = true;
            bullets[i].position = player.position;
            bullets[i].radius = BULLET_RADIUS;
            bullets[i].lifeTime = BULLET_LIFETIME;
            bullets[i].isPlayerOwned = true;
            bullets[i].velocity =
                Vector2Scale(Vector2Normalize(aimDir), BULLET_SPEED);
            break;
          }
        }
      }
    }

    // Enemy AI & Updates
    for (int i = 0; i < enemyCount; i++) {
      if (!enemies[i].active)
        continue;

      // Simple distance check activation (Wake up when close)
      if (Vector2Distance(player.position, enemies[i].position) < 800.0f) {
        enemies[i].shootTimer -= dt;
        if (enemies[i].shootTimer <= 0) {
          enemies[i].shootTimer = ENEMY_SHOOT_INTERVAL;
          // Fire at Player
          for (int b = 0; b < MAX_BULLETS; b++) {
            if (!bullets[b].active) {
              bullets[b].active = true;
              bullets[b].position = enemies[i].position;
              bullets[b].radius = BULLET_RADIUS;
              bullets[b].lifeTime = BULLET_LIFETIME;
              bullets[b].isPlayerOwned = false;

              Vector2 toPlayer =
                  Vector2Subtract(player.position, enemies[i].position);
              bullets[b].velocity =
                  Vector2Scale(Vector2Normalize(toPlayer), BULLET_SPEED * 0.6f);
              break;
            }
          }
        }
      }
    }

    // Bullet Updates & Collisions
    for (int i = 0; i < MAX_BULLETS; i++) {
      if (!bullets[i].active)
        continue;

      bullets[i].position = Vector2Add(bullets[i].position,
                                       Vector2Scale(bullets[i].velocity, dt));
      bullets[i].lifeTime -= dt;

      // Check Wall Collisions (destroy bullet)
      for (int w = 0; w < wallCount; w++) {
        if (CheckCollisionCircleRec(bullets[i].position, bullets[i].radius,
                                    walls[w])) {
          bullets[i].active = false;
        }
      }
      if (!bullets[i].active)
        continue;

      if (bullets[i].isPlayerOwned) {
        // Check against Enemies
        for (int e = 0; e < enemyCount; e++) {
          if (enemies[e].active &&
              CheckCollisionCircles(bullets[i].position, bullets[i].radius,
                                    enemies[e].position, enemies[e].radius)) {
            enemies[e].active = false;
            bullets[i].active = false;
            // Drop Mask Logic
            maskActive = true;
            droppedMask.identity = enemies[e].identity;
            droppedMask.position = enemies[e].position;
            droppedMask.radius = 15.0f;
            droppedMask.active =
                true; // Use this as validity, draw logic uses 'maskActive'
          }
        }
      } else {
        // Check against Player
        if (CheckCollisionCircles(bullets[i].position, bullets[i].radius,
                                  player.position, player.radius)) {
          gameOver = true;
          bullets[i].active = false;
        }
      }
    }

    // Mask Pickup
    if (maskActive) {
      if (CheckCollisionCircles(player.position, player.radius,
                                droppedMask.position, droppedMask.radius)) {
        if (IsKeyPressed(KEY_SPACE)) {
          player.identity = droppedMask.identity;
          maskActive = false; // Picked up
        }
      }
    }

    // Door State Logic (Visual)
    for (int i = 0; i < doorCount; i++) {
      // Open if player is nearby/overlapping AND has permission
      // Or if he has permission and is close enough to interact?
      // Visual only: Open if standing in it with permission.
      if (CheckCollisionCircleRec(player.position, player.radius, doors[i]) &&
          player.identity.permissionLevel >= doorPerms[i]) {
        doorsOpen[i] = true;
      } else {
        doorsOpen[i] = false;
      }
    }

    // Win Condition: Passed the 3rd door (X > 3060)
    if (player.position.x > 3100.0f) {
      gameWon = true;
    }

    // --- DRAW ---
    BeginDrawing();
    ClearBackground((Color){20, 20, 25, 255}); // Slightly blue-ish dark bg

    BeginMode2D(camera);

    // Draw Walls
    for (int i = 0; i < wallCount; i++)
      DrawRectangleRec(walls[i], GRAY);

    // Draw Doors
    for (int i = 0; i < doorCount; i++) {
      Color dColor = doorsOpen[i] ? GREEN : DARKGRAY;
      if (!doorsOpen[i]) {
        DrawRectangleRec(doors[i], dColor);
        DrawText(TextFormat("REQ: LVL %d", doorPerms[i]), (int)doors[i].x - 10,
                 (int)doors[i].y + 50, 10, WHITE);
      } else {
        DrawRectangleLinesEx(doors[i], 3.0f, GREEN);
      }
    }

    // Draw Level Info on Floor
    DrawText("ZONE 1: STAFF ONLY", 400, 300, 30, Fade(WHITE, 0.1f));
    DrawText("ZONE 2: GUARD ONLY", 1400, 300, 30, Fade(WHITE, 0.1f));
    DrawText("ZONE 3: ADMIN ONLY", 2400, 300, 30, Fade(WHITE, 0.1f));
    DrawText("EXIT", 3200, 300, 40, Fade(GREEN, 0.5f));

    // Draw Enemies
    for (int i = 0; i < enemyCount; i++) {
      if (enemies[i].active) {
        DrawCircleV(enemies[i].position, enemies[i].radius,
                    enemies[i].identity.color);
        DrawCircleLines((int)enemies[i].position.x, (int)enemies[i].position.y,
                        enemies[i].radius + 2, WHITE); // Highlight
      }
    }

    // Draw Mask
    if (maskActive) {
      DrawCircleV(droppedMask.position, droppedMask.radius,
                  droppedMask.identity.color);
      DrawCircleLines((int)droppedMask.position.x, (int)droppedMask.position.y,
                      droppedMask.radius + 4, GOLD);
      DrawText("PRESS SPACE", (int)droppedMask.position.x - 30,
               (int)droppedMask.position.y - 30, 10, WHITE);
    }

    // Draw Bullets
    for (int i = 0; i < MAX_BULLETS; i++) {
      if (bullets[i].active) {
        DrawCircleV(bullets[i].position, bullets[i].radius,
                    bullets[i].isPlayerOwned ? YELLOW : ORANGE);
      }
    }

    // Draw Player
    DrawCircleV(player.position, player.radius, player.identity.color);
    DrawLineV(player.position, mouseWorld, Fade(WHITE, 0.2f));

    EndMode2D();

    // HUD (Fixed on screen)
    DrawText(TextFormat("CURRENT LEVEL: %d", player.identity.permissionLevel),
             20, 20, 20, WHITE);
    if (gameWon)
      DrawText("WINNER!", SCREEN_WIDTH / 2 - 50, 100, 40, GOLD);

    EndDrawing();
  }

  CloseWindow();
  return 0;
}
