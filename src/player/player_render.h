
#include "../anim.h"
#include "../entity.h"

#ifndef PLAYER_RENDER_H
#define PLAYER_RENDER_H

// Holds all player-related visual assets and animation state.
typedef struct {
    // Feet animations
    AnimClip feetIdleClip;
    AnimClip feetWalkClip;
    AnimClip feetRunClip;
    AnimClip feetStrafeLeftClip;
    AnimClip feetStrafeRightClip;

    // Weapon animations - Handgun
    AnimClip handgunIdleClip;
    AnimClip handgunMoveClip;
    AnimClip handgunShootClip;
    AnimClip handgunReloadClip;
    AnimClip handgunMeleeClip;

    // Weapon animations - Rifle
    AnimClip rifleIdleClip;
    AnimClip rifleMoveClip;
    AnimClip rifleShootClip;
    AnimClip rifleReloadClip;
    AnimClip rifleMeleeClip;

    // Weapon animations - Shotgun
    AnimClip shotgunIdleClip;
    AnimClip shotgunMoveClip;
    AnimClip shotgunShootClip;
    AnimClip shotgunReloadClip;
    AnimClip shotgunMeleeClip;

    // Weapon animations - Flashlight
    AnimClip flashlightIdleClip;
    AnimClip flashlightMoveClip;
    AnimClip flashlightMeleeClip;

    // Weapon animations - Knife
    AnimClip knifeIdleClip;
    AnimClip knifeMoveClip;
    AnimClip knifeMeleeClip;

    // Runtime anim players
    AnimPlayer feetAnim;
    AnimPlayer weaponAnim;

    // Shadow texture
    Texture2D shadow;
    
    // Muzzle flash texture
    Texture2D muzzleFlash;

    // State
    bool loaded;
    PlayerEquipState lastEquip;
    int feetState;
    int weaponState;

    // Tuning
    float spriteScale;
    Vector2 spritePivot;
} PlayerRender;

// Small rendering utilities used by game/player rendering.
void PlayerRender_DrawFallback(Vector2 position, float radius);

// Attempts to compute a pivot (normalized 0..1) based on opaque pixels in a sprite frame.
// Returns true on success.
bool PlayerRender_TryComputePivotFromFrame(Texture2D frame,
										  unsigned char alphaThreshold,
										  Vector2 *outPivot);

void PlayerRender_Init(PlayerRender *pr);
void PlayerRender_LoadEpisodeAssets(PlayerRender *pr);
void PlayerRender_Unload(PlayerRender *pr);

// Call every frame after player movement/rotation have been updated.
void PlayerRender_Update(PlayerRender *pr, const Entity *player, PlayerEquipState currentEquip, float dt, float weaponShootTimer);

// Notify when equipment changes (so visual picks a new idle pose).
void PlayerRender_OnEquip(PlayerRender *pr, PlayerEquipState equip);

// Draw shadow + feet + weapon at player.position with player.rotation.
void PlayerRender_Draw(const PlayerRender *pr, const Entity *player, PlayerEquipState currentEquip);

// Draw muzzle flash when shooting
void PlayerRender_DrawMuzzleFlash(const PlayerRender *pr, const Entity *player, PlayerEquipState currentEquip, float weaponShootTimer);

#endif
