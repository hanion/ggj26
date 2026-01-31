#include "hud.h"

#include "../raylib/src/raylib.h"

// --- Profile portrait (top-left) ---
static Texture2D playerProfileTexture;
static bool playerProfileLoaded = false;

// --- HUD icons (next to the portrait) ---
static Texture2D hudLevelTexture;
static Texture2D hudWeaponTexture;
static Texture2D hudBulletTexture;
static bool hudIconsLoaded = false;

static void LoadHudIcons(void) {
    if (hudIconsLoaded) return;

    hudLevelTexture = LoadTexture("assets/hud/level.png");
    hudWeaponTexture = LoadTexture("assets/hud/weapon.png");
    hudBulletTexture = LoadTexture("assets/hud/bullet.png");

    // Consider icons "loaded" even if one is missing; we'll guard on .id when drawing.
    hudIconsLoaded = true;
}

static void UnloadHudIcons(void) {
    if (!hudIconsLoaded) return;

    if (hudLevelTexture.id != 0) UnloadTexture(hudLevelTexture);
    if (hudWeaponTexture.id != 0) UnloadTexture(hudWeaponTexture);
    if (hudBulletTexture.id != 0) UnloadTexture(hudBulletTexture);

    hudLevelTexture = (Texture2D){0};
    hudWeaponTexture = (Texture2D){0};
    hudBulletTexture = (Texture2D){0};
    hudIconsLoaded = false;
}

static void DrawHudIconRow(Vector2 pos, Texture2D icon, const char *text) {
    const float iconSize = 50.0f;
    const float gap = 14.0f;
    const int fontSize = 22;

    Rectangle iconDst = (Rectangle){pos.x, pos.y, iconSize, iconSize};

    if (icon.id != 0) {
        Rectangle src = (Rectangle){0, 0, (float)icon.width, (float)icon.height};
        DrawTexturePro(icon, src, iconDst, (Vector2){0, 0}, 0.0f, WHITE);
    } else {
        // Fallback if texture missing
        DrawRectangleRec(iconDst, Fade(WHITE, 0.15f));
        DrawRectangleLines((int)iconDst.x, (int)iconDst.y, (int)iconDst.width, (int)iconDst.height, Fade(WHITE, 0.25f));
    }

    DrawText(text, (int)(pos.x + iconSize + gap), (int)(pos.y + 2), fontSize, WHITE);
}

static void LoadPlayerProfile(void) {
    if (playerProfileLoaded) return;
    playerProfileTexture = LoadTexture("assets/hud/profile/player_1.png");
    playerProfileLoaded = (playerProfileTexture.id != 0);
}

static void UnloadPlayerProfile(void) {
    if (playerProfileLoaded && playerProfileTexture.id != 0) {
        UnloadTexture(playerProfileTexture);
        playerProfileTexture = (Texture2D){0};
    }
    playerProfileLoaded = false;
}

// Draws a stylized top-left portrait (triangular mask over a rectangle).
static void DrawPlayerProfileHUD(Vector2 topLeft) {
    LoadPlayerProfile();

    const float panelW = 200.0f;
    const float panelH = 155.0f;
    Rectangle panel = (Rectangle){topLeft.x, topLeft.y, panelW, panelH};

    // Panel background
    DrawRectangleRounded(panel, 0.15f, 8, Fade(BLACK, 0.55f));
    // NOTE: raylib signature is (rec, roundness, segments, color) — no thickness param.
    DrawRectangleRoundedLines(panel, 0.15f, 8, Fade(WHITE, 0.25f));

    // Portrait area inside the panel
    const float pad = 10.0f;
    Rectangle dst = (Rectangle){panel.x + pad, panel.y + pad, panelW - pad * 2.0f, panelH - pad * 2.0f};

    if (playerProfileLoaded) {
        // Compute a source rect that keeps aspect ratio while filling dst (center-crop)
        float srcW = (float)playerProfileTexture.width;
        float srcH = (float)playerProfileTexture.height;
        float srcAspect = srcW / srcH;
        float dstAspect = dst.width / dst.height;
        Rectangle src = (Rectangle){0, 0, srcW, srcH};
        if (srcAspect > dstAspect) {
            // Source is wider: crop horizontally
            float newW = srcH * dstAspect;
            src.x = (srcW - newW) * 0.5f;
            src.width = newW;
        } else {
            // Source is taller: crop vertically
            float newH = srcW / dstAspect;
            src.y = (srcH - newH) * 0.5f;
            src.height = newH;
        }

        // Raylib doesn't support arbitrary (triangle) clipping; draw full portrait into a rect
        // then cover corners with triangles to create a "triangular" feel.
        BeginScissorMode((int)dst.x, (int)dst.y, (int)dst.width, (int)dst.height);
        DrawTexturePro(playerProfileTexture, src, dst, (Vector2){0, 0}, 0.0f, WHITE);
        EndScissorMode();

        Color cover = Fade(BLACK, 0.55f);
        // Bottom-left cut
        DrawTriangle(
            (Vector2){dst.x, dst.y + dst.height},
            (Vector2){dst.x, dst.y + dst.height * 0.55f},
            (Vector2){dst.x + dst.width * 0.55f, dst.y + dst.height},
            cover);
        // Top-right cut
        DrawTriangle(
            (Vector2){dst.x + dst.width, dst.y},
            (Vector2){dst.x + dst.width * 0.55f, dst.y},
            (Vector2){dst.x + dst.width, dst.y + dst.height * 0.45f},
            cover);

        // Accent edge lines on the cuts
        DrawLineV(
            (Vector2){dst.x, dst.y + dst.height * 0.55f},
            (Vector2){dst.x + dst.width * 0.55f, dst.y + dst.height},
            Fade(WHITE, 0.25f));
        DrawLineV(
            (Vector2){dst.x + dst.width * 0.55f, dst.y},
            (Vector2){dst.x + dst.width, dst.y + dst.height * 0.45f},
            Fade(WHITE, 0.25f));
    } else {
        DrawRectangleRec(dst, Fade(DARKGRAY, 0.7f));
    DrawText("NO PROFILE", (int)dst.x + 16, (int)dst.y + 55, 18, Fade(WHITE, 0.7f));
    }
}

void Hud_Init(void) {
    playerProfileTexture = (Texture2D){0};
    playerProfileLoaded = false;

    hudLevelTexture = (Texture2D){0};
    hudWeaponTexture = (Texture2D){0};
    hudBulletTexture = (Texture2D){0};
    hudIconsLoaded = false;
}

void Hud_Shutdown(void) {
    UnloadPlayerProfile();
    UnloadHudIcons();
}

void Hud_DrawPlayer(const Entity *player) {
    if (!player) return;

    LoadHudIcons();

    const Vector2 topLeft = (Vector2){20.0f, 20.0f};
    DrawPlayerProfileHUD(topLeft);

    // Layout: portrait panel is 200x155 (see DrawPlayerProfileHUD).
    // Place icon rows to the right of portrait.
    const float portraitW = 200.0f;
    const float iconStartX = topLeft.x + portraitW + 18.0f;
    const float lineY0 = topLeft.y;
    const float lineGap = 36.0f;

    bool hasGunEquipped = (player->equipmentState == PLAYER_EQUIP_HANDGUN ||
                          player->equipmentState == PLAYER_EQUIP_RIFLE ||
                          player->equipmentState == PLAYER_EQUIP_SHOTGUN);

    // Row 1: Level icon + level text
    DrawHudIconRow((Vector2){iconStartX, lineY0 + 0.0f},
                   hudLevelTexture,
                   TextFormat("LEVEL: %d", player->identity.permissionLevel));

    // Row 2: Weapon icon + weapon text
    const char *weaponText = "WEAPON HANDS";
    switch (player->equipmentState) {
        case PLAYER_EQUIP_KNIFE: weaponText = "WEAPON KNIFE"; break;
        case PLAYER_EQUIP_FLASHLIGHT: weaponText = "WEAPON FLASHLIGHT"; break;
        case PLAYER_EQUIP_HANDGUN: weaponText = "WEAPON HANDGUN"; break;
        case PLAYER_EQUIP_RIFLE: weaponText = "WEAPON RIFLE"; break;
        case PLAYER_EQUIP_SHOTGUN: weaponText = "WEAPON SHOTGUN"; break;
        default: weaponText = "WEAPON HANDS"; break;
    }
    DrawHudIconRow((Vector2){iconStartX, lineY0 + lineGap},
                   hudWeaponTexture,
                   TextFormat("%s", weaponText));

    // Row 3: Bullet icon + ammo text (only when player has a gun)
    if (hasGunEquipped) {
        DrawHudIconRow((Vector2){iconStartX, lineY0 + lineGap * 2.0f},
                       hudBulletTexture,
                       TextFormat("BULLET: %d / %d", player->magAmmo, player->reserveAmmo));
        if (player->isReloading) {
            DrawText("RELOADING...", (int)iconStartX, (int)(lineY0 + lineGap * 3.0f + 20.0f), 22, YELLOW);
        }
    }
}
