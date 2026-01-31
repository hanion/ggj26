#ifndef LEVELS_H
#define LEVELS_H

#include "../raylib/src/raylib.h"
#include "entity.h"
#include "types.h"
#include <stddef.h>

#define MAX_WALLS 200
#define MAX_DOORS 20
#define MAX_ENEMIES 50

typedef struct {
	Texture2D texture;
	Rectangle source;
	Rectangle dest;
} Background;

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

  Background bgs[MAX_DOORS];
  size_t bgs_count;

} Level;

// Function prototype for level loader
void InitLevel(int episode, Level *level);
void UnloadLevel(int episode);

#endif // LEVELS_H
