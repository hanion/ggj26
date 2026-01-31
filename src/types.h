#ifndef TYPES_H
#define TYPES_H

#include "../raylib/src/raylib.h"

// Permission Levels (Now mostly for Cards and Doors)
typedef enum {
  PERM_NONE = 0,
  PERM_STAFF = 1,
  PERM_GUARD = 2,
  PERM_ADMIN = 3
} PermissionLevel;

// Ability Flags (Bitmask) - Keeping for legacy check or new mask system
typedef enum {
  ABILITY_NONE = 0,
  ABILITY_PUNCH = 1 << 0,
  ABILITY_SHOOT = 1 << 1,
  ABILITY_DASH = 1 << 2
} AbilityFlags;

// Enemy Types
typedef enum {
  ENEMY_CIVILIAN = 0,
  ENEMY_STAFF,
  ENEMY_GUARD,
  ENEMY_ADMIN
} EnemyType;

// NPC Types
typedef enum {
  NPC_NONE = 0,
  NPC_KIZ,
  NPC_COCUK,
  NPC_BALIKCI,
  NPC_SIGARACI
} NpcType;

// AI Behavior Types
typedef enum {
  AI_WALKER,
  AI_GUARDIAN,
  AI_NONE
} AIType;

// AI States
typedef enum {
  STATE_IDLE,
  STATE_PATROL,
  STATE_ATTACK,
  STATE_SEARCH
} EnemyState;

// --- GUN SYSTEM ---
typedef enum {
  GUN_NONE = 0,
  GUN_KNIFE,
  GUN_HANDGUN,
  GUN_RIFLE,
  GUN_SHOTGUN
} GunType;

typedef struct {
  GunType type;
  int currentAmmo;
  int maxAmmo;
  int reserveAmmo;
  float reloadTime;
  float cooldown;     // Time between shots
  float range;
  float damage;
  bool active;        // Does this slot have a gun?
} Gun;

typedef struct {
    Vector2 position;
    float radius;
    bool active;
    Gun gun;
} DroppedGun;

// --- MASK SYSTEM ---
typedef enum {
  MASK_NONE = 0,
  MASK_SPEED,         // Run faster
  MASK_STEALTH,       // Harder to see
  MASK_STRENGTH,      // Better melee?
  MASK_ADMIN          // Maybe specific abilities
} MaskAbilityType;

typedef struct {
  MaskAbilityType type;
  float maxDuration;  // Usually 10.0f
  float currentTimer; // Counts down when active
  bool isActive;      // Is the ability currently being used?
  bool isBroken;      // If duration hits 0, it breaks
  bool collected;     // Is this slot occupied?
  Color color;        // Visual representation
} Mask;

// --- PERMISSION SYSTEM ---
typedef struct {
  PermissionLevel level;
  bool collected;
} PermissionCard;

// --- INVENTORY ---
#define MAX_GUN_SLOTS 3
#define MAX_MASK_SLOTS 3

typedef struct {
  // Guns
  Gun gunSlots[MAX_GUN_SLOTS];
  int currentGunIndex; // 0, 1, 2

  // Masks
  Mask maskSlots[MAX_MASK_SLOTS];
  int currentMaskIndex; // 0, 1, 2

  // Keycard
  PermissionCard card;
} Inventory;

// Legacy "Identity" needs to be refactored or kept for Enemies temporarily
// For now, let's keep a simplified Identity for Enemies/Base Stats
typedef struct {
  PermissionLevel permissionLevel; // For enemies, innate access
  float speed;
  Color color;
} Identity;

#endif // TYPES_H
