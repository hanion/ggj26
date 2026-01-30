#ifndef ENTITY_H
#define ENTITY_H

#include "../raylib/src/raylib.h"
#include "types.h"

typedef struct {
  Vector2 position;
  Vector2 velocity;
  float rotation; // For aiming
  bool active;    // Is this entity currently in the world?

  Identity identity; // The current mask/persona equipped

  // Gameplay state
  bool isPlayer;
  float radius;     // Collision size
  float shootTimer; // For AI cooldown

  int equipmentState;
  bool hasGun;
  float aimAngle;
} Entity;

typedef struct {
  Vector2 position;
  Vector2 velocity;
  float radius;
  bool active;
  float lifeTime;     // Optional: cleanup bullets after some time
  bool isPlayerOwned; // True = Player shot, False = Enemy shot
} Bullet;

#endif // ENTITY_H
