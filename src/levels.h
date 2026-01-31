#ifndef LEVELS_H
#define LEVELS_H

#include "../raylib/src/raylib.h"
#include "entity.h"
#include "types.h"

#define MAX_WALLS 200
#define MAX_DOORS 20
#define MAX_ENEMIES 50

typedef struct {
  int id; // Phase/Episode ID

  // Level Layout
  Rectangle walls[MAX_WALLS];
  int wallCount;

  // Interactive Objects
  Rectangle doors[MAX_DOORS];
  PermissionLevel doorPerms[MAX_DOORS];
  bool doorsOpen[MAX_DOORS];
  int doorCount;

  // NPCs
  Entity enemies[MAX_ENEMIES];
  int enemyCount;

  // Spawn Point
  Vector2 playerSpawn;
  Identity playerStartId;

  // Win Condition (Reach X)
  float winX;
} Level;

// Function prototype for level loader
void InitLevel(int episode, Level *level);

#endif // LEVELS_H
