# Handgun Shooting & Reloading System Implementation

## Overview
This document describes the implementation of the handgun shooting and reloading system for the ggj26 game.

## Features Implemented

### 1. Ammo Tracking System
- **Magazine Ammo (`magAmmo`)**: Current rounds in the magazine (default: 12)
- **Reserve Ammo (`reserveAmmo`)**: Backup ammunition for reloading (default: 48)
- Both values are stored in the `Entity` struct and displayed in the HUD

### 2. Shooting Mechanics
- **Input**: Mouse left-click OR Space key
- **Ammo Consumption**: Each shot consumes 1 round from `magAmmo`
- **Restrictions**: 
  - Cannot shoot while reloading
  - Cannot shoot with empty magazine
  - Only works with gun equipment (handgun, rifle, shotgun)
- **Auto-Reload**: Attempting to shoot with empty mag triggers automatic reload (if reserve ammo available)

### 3. Reload System
- **Manual Trigger**: R key
- **Animation Duration**: 1.5 seconds (configurable via `RELOAD_DURATION`)
- **Animation**: Uses existing `survivor-reload_handgun` asset clips
- **Ammo Transfer**: 
  - Transfers ammo from reserve to magazine when reload completes
  - Fills magazine to capacity or uses all remaining reserve (whichever is less)
  - Example: 0/36 → R → (1.5s) → 12/24

### 4. Muzzle Flash
- **Asset**: `assets/better_character/Survivor Spine/images/muzzle_flash_01.png`
- **Offset Constants** (easily tweakable):
  - `MUZZLE_OFFSET_X`: 40.0f
  - `MUZZLE_OFFSET_Y`: -5.0f
- **Positioning**: Offset is rotated based on player's aim direction
- **Display**: Only shown during the shoot animation timer (0.2 seconds)

### 5. HUD Display
When equipped with a gun, the HUD shows:
- Current ammo in magazine / Reserve ammo (e.g., "AMMO: 12 / 48")
- "RELOADING..." indicator during reload animation

## Code Changes

### Files Modified
1. **src/entity.h**: Added ammo and reload state fields to `Entity` struct
2. **src/player.c**: Initialize ammo values in `InitPlayer()`
3. **src/player_render.h**: Added muzzle flash texture field
4. **src/player_render.c**: 
   - Added `PR_WEAPON_RELOAD` state
   - Implemented muzzle flash rendering with proper offset
   - Updated state machine to handle reload animation
5. **src/game.c**:
   - Added ammo constants (`MAG_SIZE`, `RESERVE_AMMO_START`, `RELOAD_DURATION`)
   - Implemented reload timer logic
   - Updated shooting to consume ammo and respect reload state
   - Added R key reload trigger
   - Added ammo HUD display
   - Integrated muzzle flash rendering

## Constants (Easy to Tweak)

```c
// In game.c
#define MAG_SIZE 12                  // Magazine capacity
#define RESERVE_AMMO_START 48        // Starting reserve ammo
#define RELOAD_DURATION 1.5f         // Reload animation duration

// In player_render.c
#define MUZZLE_OFFSET_X 40.0f        // Forward offset from player center
#define MUZZLE_OFFSET_Y -5.0f        // Vertical offset from player center
```

## Testing
All core ammo logic has been validated:
- ✓ Shooting consumes ammo correctly
- ✓ Reload transfers ammo from reserve to magazine
- ✓ Partial reloads work correctly
- ✓ Reload with insufficient reserve ammo handled
- ✓ Auto-reload triggers when shooting with empty mag

## Usage in Game
1. **Equip handgun** (auto-equipped after first kill)
2. **Shoot**: Left-click or Space
3. **Reload**: R key (when magazine not full and reserve > 0)
4. **Monitor ammo**: Check HUD for current/reserve ammo counts
5. **Watch for "RELOADING..."** indicator during reload

## Implementation Philosophy
The implementation follows the requirement to keep it "lightweight and easy to tweak":
- Simple constants for all tunable values
- Straightforward logic without over-engineering
- Minimal code changes to existing systems
- Easy to understand and modify
