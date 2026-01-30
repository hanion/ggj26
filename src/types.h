#ifndef TYPES_H
#define TYPES_H

#include "../raylib/src/raylib.h"

// Permission Levels for Doors/Zones
typedef enum {
  PERM_CIVILIAN = 0,
  PERM_STAFF = 1,
  PERM_GUARD = 2,
  PERM_ADMIN = 3
} PermissionLevel;

// Ability Flags (Bitmask)
typedef enum {
  ABILITY_NONE = 0,
  ABILITY_PUNCH = 1 << 0,
  ABILITY_SHOOT = 1 << 1,
  ABILITY_DASH = 1 << 2
} AbilityFlags;

// The "Mask" or "Identity" that defines what an entity can do
typedef struct {
  PermissionLevel permissionLevel;
  int abilities; // Bitwise combination of AbilityFlags
  Color color;   // Visual representation (The Mask visual)
  float speed;   // Movement speed modifier (optional but good for gameplay)
} Identity;

#endif // TYPES_H
