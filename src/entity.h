#ifndef ENTITY_H
#define ENTITY_H

#include "../raylib/src/raylib.h"
#include "types.h"

typedef enum {
  PLAYER_EQUIP_BARE_HANDS,
  PLAYER_EQUIP_KNIFE,
  PLAYER_EQUIP_FLASHLIGHT,
  PLAYER_EQUIP_HANDGUN,
  PLAYER_EQUIP_RIFLE,
  PLAYER_EQUIP_SHOTGUN
} PlayerEquipState;

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

  PlayerEquipState equipmentState;
  
  // Ammo system
  int magAmmo;      // Current ammo in magazine
  int reserveAmmo;  // Reserve ammo for reloading
  bool isReloading; // Is currently reloading
  float reloadTimer; // Timer for reload animation

  // AI State
  AIType aiType;
  EnemyState state;
  float sightRange;
  float sightAngle;       // Degrees, full cone (e.g. 120)
  Vector2 lastKnownPlayerPos;
  float searchTimer;      // How long to search/investigate
  Vector2 patrolStart;    // Home position for patrolling
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
