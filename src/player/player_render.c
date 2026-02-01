#include "player_render.h"

#include <stddef.h>
#include "../../raylib/src/raymath.h"

// Keep these local so game.c stays clean.
typedef enum {
    PR_FEET_IDLE,
    PR_FEET_WALK,
} PRFeetState;

typedef enum {
    PR_WEAPON_IDLE,
    PR_WEAPON_MOVE,
    PR_WEAPON_SHOOT,
    PR_WEAPON_RELOAD,
} PRWeaponState;

void PlayerRender_DrawFallback(Vector2 position, float radius) {
    float size = radius * 2.0f;
    DrawRectangleV((Vector2){position.x - radius, position.y - radius},
                   (Vector2){size, size}, RED);
}


bool PlayerRender_TryComputePivotFromFrame(Texture2D frame,
                                          unsigned char alphaThreshold,
                                          Vector2 *outPivot) {
    if (!outPivot || frame.id == 0) {
        return false;
    }

    Image image = LoadImageFromTexture(frame);
    if (image.data == NULL) {
        TraceLog(LOG_WARNING, "Failed to read player frame for pivot");
        return false;
    }

    if (image.format != PIXELFORMAT_UNCOMPRESSED_R8G8B8A8) {
        ImageFormat(&image, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);
    }

    unsigned char *pixels = (unsigned char *)image.data;
    int minX = image.width;
    int maxX = -1;
    int minY = image.height;
    int maxY = -1;
    bool found = false;

    for (int y = 0; y < image.height; y++) {
        for (int x = 0; x < image.width; x++) {
            int index = (y * image.width + x) * 4 + 3;
            unsigned char alpha = pixels[index];
            if (alpha > alphaThreshold) {
                if (x < minX) minX = x;
                if (x > maxX) maxX = x;
                if (y < minY) minY = y;
                if (y > maxY) maxY = y;
                found = true;
            }
        }
    }

    if (found) {
        float centerX = (minX + maxX) * 0.5f;
        float centerY = (minY + maxY) * 0.5f;
        *outPivot = (Vector2){centerX / (float)image.width,
                              centerY / (float)image.height};
    } else {
        TraceLog(LOG_WARNING, "Player frame had no opaque pixels for pivot");
    }

    UnloadImage(image);
    return found;
}


static void DrawFrame(Texture2D frame, Vector2 position, float rotation, float scale, Vector2 pivot) {
    Rectangle source = (Rectangle){0.0f, 0.0f, (float)frame.width, (float)frame.height};

    float destW = frame.width * scale;
    float destH = frame.height * scale;

    Rectangle dest = (Rectangle){position.x, position.y, destW, destH};
    Vector2 origin = (Vector2){destW * pivot.x, destH * pivot.y};

    DrawTexturePro(frame, source, dest, origin, rotation, WHITE);
}

static void DrawShadow(Texture2D shadow, Vector2 position, float scale, Vector2 pivot) {
    Rectangle source = (Rectangle){0.0f, 0.0f, (float)shadow.width, (float)shadow.height};

    float destW = shadow.width * scale;
    float destH = shadow.height * scale;

    Rectangle dest = (Rectangle){position.x, position.y, destW, destH};
    Vector2 origin = (Vector2){destW * pivot.x, destH * pivot.y};

    DrawTexturePro(shadow, source, dest, origin, 0.0f, WHITE);
}

static AnimClip *GetWeaponClip(PlayerRender *pr, PlayerEquipState equip, PRWeaponState state) {
    switch (equip) {
        case PLAYER_EQUIP_HANDGUN:
            return state == PR_WEAPON_RELOAD ? &pr->handgunReloadClip :
                   state == PR_WEAPON_SHOOT ? &pr->handgunShootClip :
                   state == PR_WEAPON_MOVE ? &pr->handgunMoveClip :
                                             &pr->handgunIdleClip;
        case PLAYER_EQUIP_RIFLE:
            return state == PR_WEAPON_RELOAD ? &pr->rifleReloadClip :
                   state == PR_WEAPON_SHOOT ? &pr->rifleShootClip :
                   state == PR_WEAPON_MOVE ? &pr->rifleMoveClip :
                                             &pr->rifleIdleClip;
        case PLAYER_EQUIP_SHOTGUN:
            return state == PR_WEAPON_RELOAD ? &pr->shotgunReloadClip :
                   state == PR_WEAPON_SHOOT ? &pr->shotgunShootClip :
                   state == PR_WEAPON_MOVE ? &pr->shotgunMoveClip :
                                             &pr->shotgunIdleClip;
        case PLAYER_EQUIP_FLASHLIGHT:
            // Map "shoot" request to melee for flashlight.
            return state == PR_WEAPON_SHOOT ? &pr->flashlightMeleeClip :
                   state == PR_WEAPON_MOVE ? &pr->flashlightMoveClip :
                                             &pr->flashlightIdleClip;
        case PLAYER_EQUIP_KNIFE:
            // Map "shoot" request to melee for knife.
            return state == PR_WEAPON_SHOOT ? &pr->knifeMeleeClip :
                   state == PR_WEAPON_MOVE ? &pr->knifeMoveClip :
                                             &pr->knifeIdleClip;
        default:
            return NULL;
    }
}

void PlayerRender_Init(PlayerRender *pr) {
    *pr = (PlayerRender){0};
    pr->spriteScale = 0.3f;
    pr->spritePivot = (Vector2){0.5f, 0.5f};
    pr->feetState = PR_FEET_IDLE;
    pr->weaponState = PR_WEAPON_IDLE;
    pr->lastEquip = PLAYER_EQUIP_BARE_HANDS;
}

void PlayerRender_LoadEpisodeAssets(PlayerRender *pr) {
    if (pr->loaded) {
        PlayerRender_Unload(pr);
    }

    pr->feetIdleClip = LoadAnimClip("assets/better_character/feet/idle", 30.0f);
    pr->feetWalkClip = LoadAnimClip("assets/better_character/feet/walk", 30.0f);
    pr->feetRunClip = LoadAnimClip("assets/better_character/feet/run", 30.0f);
    pr->feetStrafeLeftClip = LoadAnimClip("assets/better_character/feet/strafe_left", 30.0f);
    pr->feetStrafeRightClip = LoadAnimClip("assets/better_character/feet/strafe_right", 30.0f);

    pr->handgunIdleClip = LoadAnimClip("assets/better_character/handgun/idle", 30.0f);
    pr->handgunMoveClip = LoadAnimClip("assets/better_character/handgun/move", 30.0f);
    pr->handgunShootClip = LoadAnimClip("assets/better_character/handgun/shoot", 60.0f);
    pr->handgunReloadClip = LoadAnimClip("assets/better_character/handgun/reload", 30.0f);
    pr->handgunMeleeClip = LoadAnimClip("assets/better_character/handgun/meleeattack", 30.0f);

    pr->rifleIdleClip = LoadAnimClip("assets/better_character/rifle/idle", 30.0f);
    pr->rifleMoveClip = LoadAnimClip("assets/better_character/rifle/move", 30.0f);
    pr->rifleShootClip = LoadAnimClip("assets/better_character/rifle/shoot", 60.0f);
    pr->rifleReloadClip = LoadAnimClip("assets/better_character/rifle/reload", 30.0f);
    pr->rifleMeleeClip = LoadAnimClip("assets/better_character/rifle/meleeattack", 30.0f);

    // Shotgun placeholder uses rifle clips.
    pr->shotgunIdleClip = LoadAnimClip("assets/better_character/rifle/idle", 30.0f);
    pr->shotgunMoveClip = LoadAnimClip("assets/better_character/rifle/move", 30.0f);
    pr->shotgunShootClip = LoadAnimClip("assets/better_character/rifle/shoot", 60.0f);
    pr->shotgunReloadClip = LoadAnimClip("assets/better_character/rifle/reload", 30.0f);
    pr->shotgunMeleeClip = LoadAnimClip("assets/better_character/rifle/meleeattack", 30.0f);

    pr->flashlightIdleClip = LoadAnimClip("assets/better_character/flashlight/idle", 30.0f);
    pr->flashlightMoveClip = LoadAnimClip("assets/better_character/flashlight/move", 30.0f);
    pr->flashlightMeleeClip = LoadAnimClip("assets/better_character/flashlight/meleeattack", 30.0f);

    pr->knifeIdleClip = LoadAnimClip("assets/better_character/knife/idle", 30.0f);
    pr->knifeMoveClip = LoadAnimClip("assets/better_character/knife/move", 30.0f);
    pr->knifeMeleeClip = LoadAnimClip("assets/better_character/knife/meleeattack", 30.0f);

    pr->shadow = LoadTexture("assets/better_character/shadow.png");
    pr->muzzleFlash = LoadTexture("assets/better_character/Survivor Spine/images/muzzle_flash_01-removebg-preview.png");

    pr->feetAnim = (AnimPlayer){0};
    pr->weaponAnim = (AnimPlayer){0};

    AnimPlayer_SetClip(&pr->feetAnim, &pr->feetIdleClip);
    AnimPlayer_SetClip(&pr->weaponAnim, &pr->knifeIdleClip);

    pr->feetState = PR_FEET_IDLE;
    pr->weaponState = PR_WEAPON_IDLE;

    pr->loaded = pr->feetIdleClip.frame_count > 0 && pr->feetWalkClip.frame_count > 0;
}

void PlayerRender_Unload(PlayerRender *pr) {
    if (!pr) return;

    UnloadAnimClip(&pr->feetIdleClip);
    UnloadAnimClip(&pr->feetWalkClip);
    UnloadAnimClip(&pr->feetRunClip);
    UnloadAnimClip(&pr->feetStrafeLeftClip);
    UnloadAnimClip(&pr->feetStrafeRightClip);

    UnloadAnimClip(&pr->handgunIdleClip);
    UnloadAnimClip(&pr->handgunMoveClip);
    UnloadAnimClip(&pr->handgunShootClip);
    UnloadAnimClip(&pr->handgunReloadClip);
    UnloadAnimClip(&pr->handgunMeleeClip);

    UnloadAnimClip(&pr->rifleIdleClip);
    UnloadAnimClip(&pr->rifleMoveClip);
    UnloadAnimClip(&pr->rifleShootClip);
    UnloadAnimClip(&pr->rifleReloadClip);
    UnloadAnimClip(&pr->rifleMeleeClip);

    UnloadAnimClip(&pr->shotgunIdleClip);
    UnloadAnimClip(&pr->shotgunMoveClip);
    UnloadAnimClip(&pr->shotgunShootClip);
    UnloadAnimClip(&pr->shotgunReloadClip);
    UnloadAnimClip(&pr->shotgunMeleeClip);

    UnloadAnimClip(&pr->flashlightIdleClip);
    UnloadAnimClip(&pr->flashlightMoveClip);
    UnloadAnimClip(&pr->flashlightMeleeClip);

    UnloadAnimClip(&pr->knifeIdleClip);
    UnloadAnimClip(&pr->knifeMoveClip);
    UnloadAnimClip(&pr->knifeMeleeClip);

    if (pr->shadow.id != 0) {
        UnloadTexture(pr->shadow);
        pr->shadow = (Texture2D){0};
    }
    
    if (pr->muzzleFlash.id != 0) {
        UnloadTexture(pr->muzzleFlash);
        pr->muzzleFlash = (Texture2D){0};
    }

    pr->loaded = false;
}

void PlayerRender_OnEquip(PlayerRender *pr, PlayerEquipState equip) {
    if (!pr || !pr->loaded) return;

    // Snap to idle clip for the new equipment.
    AnimClip *idle = GetWeaponClip(pr, equip, PR_WEAPON_IDLE);
    if (idle) {
        AnimPlayer_SetClip(&pr->weaponAnim, idle);
    }

    pr->weaponState = PR_WEAPON_IDLE;
    pr->lastEquip = equip;
}

void PlayerRender_Update(PlayerRender *pr, const Entity *player, PlayerEquipState currentEquip, float dt, float weaponShootTimer) {
    if (!pr || !pr->loaded || !player) return;

    // Feet state
    PRFeetState nextFeet = Vector2Length(player->velocity) > 0.01f ? PR_FEET_WALK : PR_FEET_IDLE;
    if (nextFeet != pr->feetState) {
        pr->feetState = nextFeet;
        AnimPlayer_SetClip(&pr->feetAnim, pr->feetState == PR_FEET_WALK ? &pr->feetWalkClip : &pr->feetIdleClip);
    }
    AnimPlayer_Update(&pr->feetAnim, dt);

    // Equipment change
    if (currentEquip != pr->lastEquip) {
        PlayerRender_OnEquip(pr, currentEquip);
    }

    if (currentEquip == PLAYER_EQUIP_BARE_HANDS) {
        return;
    }

    // Weapon state
    PRWeaponState nextWeapon = PR_WEAPON_IDLE;
    if (player->isReloading) {
        nextWeapon = PR_WEAPON_RELOAD;
    } else if (weaponShootTimer > 0.0f) {
        nextWeapon = PR_WEAPON_SHOOT;
    } else if (Vector2Length(player->velocity) > 0.01f) {
        nextWeapon = PR_WEAPON_MOVE;
    }

    if (nextWeapon != pr->weaponState) {
        pr->weaponState = nextWeapon;
        AnimClip *clip = GetWeaponClip(pr, currentEquip, pr->weaponState);
        if (clip) {
            AnimPlayer_SetClip(&pr->weaponAnim, clip);
            // Don't loop reload animation, but loop all others
            // Also don't loop melee/shoot
            bool loop = (nextWeapon != PR_WEAPON_RELOAD && nextWeapon != PR_WEAPON_SHOOT);
             // Move/Idle loop. Shoot/Reload/Melee don't.
             if (nextWeapon == PR_WEAPON_IDLE || nextWeapon == PR_WEAPON_MOVE) loop = true;
             else loop = false;
             
            pr->weaponAnim.loop = loop;
        }
    }

    AnimPlayer_Update(&pr->weaponAnim, dt);
}

void PlayerRender_Draw(const PlayerRender *pr, const Entity *player, PlayerEquipState currentEquip) {
    if (!pr || !pr->loaded || !player) return;

    Texture2D feetFrame = AnimPlayer_GetFrame(&pr->feetAnim);
    Texture2D weaponFrame = (Texture2D){0};

    if (currentEquip != PLAYER_EQUIP_BARE_HANDS) {
        weaponFrame = AnimPlayer_GetFrame(&pr->weaponAnim);
    }

    if (pr->shadow.id != 0 && feetFrame.id != 0) {
        DrawShadow(pr->shadow, player->position, pr->spriteScale, pr->spritePivot);
    }

    if (feetFrame.id != 0) {
        DrawFrame(feetFrame, player->position, player->rotation, pr->spriteScale, pr->spritePivot);
    }

    if (weaponFrame.id != 0) {
        DrawFrame(weaponFrame, player->position, player->rotation, pr->spriteScale, pr->spritePivot);
    }
}

// Muzzle flash offset constants (easy to tweak)
#define MUZZLE_OFFSET_X 40.0f
#define MUZZLE_OFFSET_Y 16.0f
#define MUZZLE_FLASH_DURATION 0.1f  // Only show muzzle flash for this duration (seconds)

void PlayerRender_DrawMuzzleFlash(const PlayerRender *pr, const Entity *player, PlayerEquipState currentEquip, float weaponShootTimer, float weaponCooldown) {
    if (!pr || !pr->loaded || !player) return;
    if (pr->muzzleFlash.id == 0) return;
    if (weaponShootTimer <= 0.0f) return;
    
    // Only show muzzle flash for the first MUZZLE_FLASH_DURATION seconds after shooting
    float timeSinceShot = weaponCooldown - weaponShootTimer;
    if (timeSinceShot > MUZZLE_FLASH_DURATION) return;
    
    // Only show muzzle flash for guns
    if (currentEquip != PLAYER_EQUIP_HANDGUN &&
        currentEquip != PLAYER_EQUIP_RIFLE &&
        currentEquip != PLAYER_EQUIP_SHOTGUN) {
        return;
    }
    
    // Calculate muzzle flash position based on player rotation
    float rotRad = player->rotation * DEG2RAD;
    float cosRot = cosf(rotRad);
    float sinRot = sinf(rotRad);
    
    // Apply offset with rotation
    float offsetX = MUZZLE_OFFSET_X * cosRot - MUZZLE_OFFSET_Y * sinRot;
    float offsetY = MUZZLE_OFFSET_X * sinRot + MUZZLE_OFFSET_Y * cosRot;
    
    Vector2 flashPos = {
        player->position.x + offsetX,
        player->position.y + offsetY
    };
    
    // Draw the muzzle flash
    float flashScale = pr->spriteScale * 0.5f;  // Adjust scale as needed
    DrawFrame(pr->muzzleFlash, flashPos, player->rotation, flashScale, (Vector2){0.5f, 0.5f});
}

