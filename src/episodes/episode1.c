#include "../entity.h"
#include "../types.h"
#include "episodes.h"


// Define constants locally if needed, or share via header.
// For now, hardcoded values from main.c are fine or we can redefine macros.
#define ENEMY_SHOOT_INTERVAL 2.0f

void InitEpisode1(Level *level) {
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

  level->id = 1;
  level->playerSpawn = (Vector2){100.0f, 360.0f};
  level->playerStartId = idCivilian;
  level->winX = 3100.0f;

  level->wallCount = 0;
  // Walls
  level->walls[level->wallCount++] = (Rectangle){-50, -50, 4100, 50}; // Top
  level->walls[level->wallCount++] = (Rectangle){-50, 720, 4100, 50}; // Bottom
  level->walls[level->wallCount++] = (Rectangle){-50, 0, 50, 720};    // Start
  level->walls[level->wallCount++] = (Rectangle){4000, 0, 50, 720};   // End

  // Dividers
  level->walls[level->wallCount++] = (Rectangle){1000, 0, 50, 300};
  level->walls[level->wallCount++] = (Rectangle){1000, 420, 50, 300};
  level->walls[level->wallCount++] = (Rectangle){2000, 0, 50, 300};
  level->walls[level->wallCount++] = (Rectangle){2000, 420, 50, 300};
  level->walls[level->wallCount++] = (Rectangle){3000, 0, 50, 300};
  level->walls[level->wallCount++] = (Rectangle){3000, 420, 50, 300};

  // Doors
  level->doors[0] = (Rectangle){1000, 300, 50, 120};
  level->doorPerms[0] = PERM_STAFF;
  level->doorsOpen[0] = false;
  level->doors[1] = (Rectangle){2000, 300, 50, 120};
  level->doorPerms[1] = PERM_GUARD;
  level->doorsOpen[1] = false;
  level->doors[2] = (Rectangle){3000, 300, 50, 120};
  level->doorPerms[2] = PERM_ADMIN;
  level->doorsOpen[2] = false;
  level->doorCount = 3;

  // Enemies
  level->enemies[0] = (Entity){.position = {800, 360},
                               .active = true,
                               .isPlayer = false,
                               .radius = 20,
                               .identity = idStaff,
                               .shootTimer = ENEMY_SHOOT_INTERVAL};
  level->enemies[1] = (Entity){.position = {1800, 360},
                               .active = true,
                               .isPlayer = false,
                               .radius = 20,
                               .identity = idGuard,
                               .shootTimer = ENEMY_SHOOT_INTERVAL};
  level->enemies[2] = (Entity){.position = {2800, 360},
                               .active = true,
                               .isPlayer = false,
                               .radius = 20,
                               .identity = idAdmin,
                               .shootTimer = ENEMY_SHOOT_INTERVAL};
  level->enemyCount = 3;
}
