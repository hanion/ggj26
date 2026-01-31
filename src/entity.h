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

  Identity identity; // Base stats (speed, color)
  Inventory inventory; // NEW: Inventory System

  // Gameplay state
  bool isPlayer;
  float radius;     // Collision size
  float shootTimer; // For AI cooldown / Gun cooldown refactor?
                    // Actually guns have their own cooldowns now in Gun struct. 
                    // But we might need a global "can fire" timer for the entity animation.

  // Removing obsolete fields now that we have Inventory:
  // PlayerEquipState equipmentState; 
  // int magAmmo;      
  // int reserveAmmo;  
  // bool isReloading; 
  // float reloadTimer; 

  // Keeping reload state here or moving to Gun? 
  // Gun struct has reloadTime. Inventory manages ammo. 
  // Let's keep a "reloading" flag on the Entity for animation state?
  bool isReloading;
  float reloadTimer; 

  // AI State
  AIType aiType;
  EnemyState state;
  float sightRange;
  float sightAngle;       // Degrees, full cone (e.g. 120)
  Vector2 lastKnownPlayerPos;
  float searchTimer;      // How long to search/investigate
  Vector2 patrolStart;    // Home position for patrolling

  bool isEnemy;
  bool isInteractive;
  NpcType npcType;
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
