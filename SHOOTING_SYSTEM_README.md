# Shooting and Reloading System

This document describes the handgun shooting and reloading system implementation.

## Features

### Ammo System
- **Magazine Capacity**: 12 rounds (configurable via `MAG_SIZE`)
- **Reserve Ammo**: 48 rounds (configurable via `INITIAL_RESERVE_AMMO`)
- Shooting consumes 1 round from the magazine
- Empty magazine prevents shooting until reloaded

### Shooting Mechanics
- **Fire Button**: Left Mouse Button or SPACE key
- **Muzzle Flash**: Visual feedback with directional offset
  - Offset from player position: `MUZZLE_OFFSET_X` = 40.0f, `MUZZLE_OFFSET_Y` = -10.0f
  - Flash duration: 0.05 seconds
  - Automatically rotates with aim direction
- Cannot shoot while reloading
- Cannot shoot with empty magazine

### Reload System
- **Reload Key**: R
- **Auto-Reload**: Automatically triggers when trying to shoot with empty magazine
- Reload animation plays during reload (currently using idle animation as placeholder)
- Ammo transfer occurs at 70% through reload animation
- Transfers ammo from reserve to magazine (up to magazine capacity)
- Cannot reload if reserve is empty
- Cannot shoot during reload

### HUD Display
- Shows current magazine ammo and reserve ammo: `AMMO: X / Y`
- Shows "RELOADING..." indicator during reload

## Configuration Constants

All constants are defined in `src/game.c` and can be easily tweaked:

```c
#define MAG_SIZE 12                      // Magazine capacity
#define INITIAL_RESERVE_AMMO 48          // Starting reserve ammo
#define MUZZLE_OFFSET_X 40.0f            // Horizontal offset for muzzle flash
#define MUZZLE_OFFSET_Y -10.0f           // Vertical offset for muzzle flash
#define SPRITE_ROTATION_OFFSET -90.0f    // Sprite orientation adjustment

static const float muzzleFlashDuration = 0.05f;   // Flash visibility duration
static const float reloadTransferFrame = 0.7f;    // When ammo transfers (70% through animation)
```

## Assets

### Muzzle Flash
- **File**: `assets/effects/muzzle_flash_01.tga`
- **Format**: TGA (Targa) image with transparency
- **Size**: 64x64 pixels
- Simple orange/yellow gradient flash effect

### Reload Animation (TODO)
- Currently uses idle animation as placeholder
- Dedicated reload animation should be placed in:
  `assets/character/LightArtilleryRobot/reloadHandgun20/`
- Animation frames should follow the naming convention used by other animations

## Code Changes

### Modified Files
1. **src/entity.h**
   - Added `magAmmo` field to Entity struct
   - Added `reserveAmmo` field to Entity struct

2. **src/game.c**
   - Added ammo constants and configuration
   - Added `PLAYER_GUN_RELOAD` animation state
   - Added muzzle flash texture and timer
   - Added reload animation clip support
   - Modified shooting logic to check and consume ammo
   - Added reload input handling (R key)
   - Added auto-reload on empty magazine
   - Added ammo transfer during reload animation
   - Added muzzle flash rendering with directional offset
   - Updated HUD to show ammo count and reload status

3. **assets/effects/muzzle_flash_01.tga** (new)
   - Muzzle flash visual effect

## Testing Checklist

- [x] Player can shoot with gun equipped
- [x] Ammo decreases when shooting
- [x] Cannot shoot when magazine is empty
- [x] Muzzle flash appears when shooting
- [x] Muzzle flash rotates with aim direction
- [x] Can reload with R key
- [x] Auto-reload triggers on empty magazine shoot attempt
- [x] Cannot shoot during reload
- [x] Ammo transfers from reserve to magazine
- [x] Reload does nothing when reserve is empty
- [x] HUD displays correct ammo count
- [x] HUD shows reload indicator

## Future Improvements

1. Add dedicated reload animation assets
2. Add reload sound effect
3. Add shooting sound effect
4. Add empty magazine "click" sound
5. Consider adding partial reload cancellation
6. Add reload speed modifiers based on identity/abilities
