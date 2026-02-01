#ifndef LEVELS_H
#define LEVELS_H

#include "../raylib/src/raylib.h"
#include "entity.h"
#include "types.h"
#include <stddef.h>

#define NPC_MAX_FRAMES 6

typedef struct NPC {
    Vector2 position;
    float radius;
    float scale; // draw scale (1.0f = original size)
    Color tint;
    const char *name;
    Texture2D frames[NPC_MAX_FRAMES];
    int frameIndex;
    int frameCount;
    float frameTimer;
    float frameDuration;
} NPC;

#define MAX_WALLS 200
#define MAX_DOORS 20
#define MAX_ENEMIES 50

typedef struct {
	Texture2D texture;
	Rectangle source;
	Rectangle dest;
} Background;

typedef struct {
    Rectangle rect;
    PermissionLevel requiredPerm;
    bool isOpen;
    float animationProgress; // 0.0 = Closed, 1.0 = Fully Open
} Door;

typedef struct {
  int id; // Phase/Episode ID

  // Level Layout
  Rectangle walls[MAX_WALLS];
  int wallCount;

  // Interactive Objects
  Door doors[MAX_DOORS];
  int doorCount;

  // NPCs
  Entity enemies[MAX_ENEMIES];
  int enemyCount;

  // NPCs (non-hostile, interactable)
  int npcCount;
  NPC npcs[8];

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
