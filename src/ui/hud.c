#include "hud.h"

#include "../../raylib/src/raylib.h"

// --- Profile portrait (top-left) ---
static Texture2D playerProfileTexture;
static bool playerProfileLoaded = false;

// --- HUD icons (next to the portrait) ---
// --- HUD icons (next to the portrait) ---
static Texture2D hudLevelTexture;
static Texture2D hudWeaponTexture;
static Texture2D hudBulletTexture;
static Texture2D texKnife;
static Texture2D texHandgun;
static Texture2D texRifle;
static bool hudIconsLoaded = false;

static void LoadHudIcons(void) {
    if (hudIconsLoaded) return;

    hudLevelTexture = LoadTexture("assets/hud/level.png");
    hudWeaponTexture = LoadTexture("assets/hud/weapon.png");
    hudBulletTexture = LoadTexture("assets/hud/bullet.png");
    texKnife = LoadTexture("assets/better_character/knife/idle/survivor-idle_knife_0.png");
    texHandgun = LoadTexture("assets/better_character/handgun/idle/survivor-idle_handgun_0.png");
    texRifle = LoadTexture("assets/better_character/rifle/idle/survivor-idle_rifle_0.png");

    // Consider icons "loaded" even if one is missing; we'll guard on .id when drawing.
    hudIconsLoaded = true;
}

static void UnloadHudIcons(void) {
    if (!hudIconsLoaded) return;

    if (hudLevelTexture.id != 0) UnloadTexture(hudLevelTexture);
    if (hudWeaponTexture.id != 0) UnloadTexture(hudWeaponTexture);
    if (hudBulletTexture.id != 0) UnloadTexture(hudBulletTexture);
    if (texKnife.id != 0) UnloadTexture(texKnife);
    if (texHandgun.id != 0) UnloadTexture(texHandgun);
    if (texRifle.id != 0) UnloadTexture(texRifle);

    hudLevelTexture = (Texture2D){0};
    hudWeaponTexture = (Texture2D){0};
    hudBulletTexture = (Texture2D){0};
    texKnife = (Texture2D){0};
    texHandgun = (Texture2D){0};
    texRifle = (Texture2D){0};
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
        //BeginScissorMode((int)dst.x, (int)dst.y, (int)dst.width, (int)dst.height);
        DrawTexturePro(playerProfileTexture, src, dst, (Vector2){0, 0}, 0.0f, WHITE);
        //EndScissorMode();

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
    const Inventory *inv = &player->inventory;

    LoadHudIcons();

    const Vector2 topLeft = (Vector2){20.0f, 20.0f};
    DrawPlayerProfileHUD(topLeft);

    // Layout: portrait panel is 200x155 (see DrawPlayerProfileHUD).
    // Place icon rows to the right of portrait.
    const float portraitW = 200.0f;
    const float iconStartX = topLeft.x + portraitW + 18.0f;
    
    // Draw Gun List below profile
    float gunListY = topLeft.y + 155.0f + 10.0f; // Below the panel
    // DrawText("INVENTORY:", (int)topLeft.x, (int)gunListY, 18, LIGHTGRAY); // Optional header
    // gunListY += 20.0f;

    for (int i = 0; i < MAX_GUN_SLOTS; i++) {
        const Gun *g = &inv->gunSlots[i];
        
        // Determine properties
        bool have = (g->type != GUN_NONE);
        Color textColor = have ? WHITE : DARKGRAY;
        if (i == inv->currentGunIndex && have) textColor = GOLD; // Highlight selected

        const char *name = "EMPTY";
        Texture2D icon = {0};
        
        if (have) {
             switch (g->type) {
                case GUN_KNIFE: name = "KNIFE"; icon = texKnife; break;
                case GUN_HANDGUN: name = "HANDGUN"; icon = texHandgun; break;
                case GUN_RIFLE: name = "RIFLE"; icon = texRifle; break;
                case GUN_SHOTGUN: name = "SHOTGUN"; icon = texRifle; break; // Placeholder
                default: name = "UNKNOWN"; break;
            }
        } 

        // Draw Format: "1) [ICON] NAME"
        // Text "1) "
        DrawText(TextFormat("%d)", i + 1), (int)topLeft.x + 10, (int)gunListY + 12, 20, textColor);
        
        // Icon (if exists)
        if (icon.id != 0) {
             // Crop center of player sprite or just draw it?
             // These are full player sprites (256x256 typically). We need to scale/crop.
             // Let's assume we maintain aspect ratio but fit in small box.
             float scale = 0.2f;
             // DrawTextureEx(icon, (Vector2){topLeft.x + 35, gunListY - 10}, 0.0f, scale, WHITE);
             // Better: Crop header/feet? The icons are full body idle.
             // We'll just draw the whole thing scaled down.
             Rectangle src = {0, 0, (float)icon.width, (float)icon.height};
             Rectangle dst = {topLeft.x + 40, gunListY, 40, 40};
             // DrawTexturePro(icon, src, dst, (Vector2){0,0}, 0.0f, have ? WHITE : Fade(WHITE, 0.3f));
             
             // Just draw a slice? No, scaling is safer.
             // Center alignment
             float aspect = src.width / src.height;
             if (aspect > 1) { dst.height = dst.width / aspect; }
             else { dst.width = dst.height * aspect; }
             
             DrawTexturePro(icon, src, dst, (Vector2){0,0}, 0.0f, have ? WHITE : Fade(BLACK, 0.5f));
        }

        // Name
        DrawText(name, (int)topLeft.x + 90, (int)gunListY + 12, 20, textColor);
        
        gunListY += 45.0f;
    }

    const float lineY0 = topLeft.y;
    const float lineGap = 36.0f;

    const Gun *currentGun = &inv->gunSlots[inv->currentGunIndex];
    
    bool hasGunEquipped = (currentGun->type != GUN_NONE && currentGun->type != GUN_KNIFE);

    // Row 1: Level/Card icon + level text
    DrawHudIconRow((Vector2){iconStartX, lineY0 + 0.0f},
                   hudLevelTexture,
                   TextFormat("ACCESS: %d", inv->card.level));

    // Row 2: Weapon icon + weapon text
    const char *weaponText = "HANDS";
    switch (currentGun->type) {
        case GUN_KNIFE: weaponText = "KNIFE"; break;
        case GUN_HANDGUN: weaponText = "HANDGUN"; break;
        case GUN_RIFLE: weaponText = "RIFLE"; break;
        case GUN_SHOTGUN: weaponText = "SHOTGUN"; break;
        default: weaponText = "HANDS"; break;
    }
    // Indicate slot
    DrawHudIconRow((Vector2){iconStartX, lineY0 + lineGap},
                   hudWeaponTexture,
                   TextFormat("[%d] %s", inv->currentGunIndex + 1, weaponText));

    // Row 3: Bullet icon + ammo text (only when player has a gun/knife?)
    // Show ammo for guns
    if (hasGunEquipped) {
        DrawHudIconRow((Vector2){iconStartX, lineY0 + lineGap * 2.0f},
                       hudBulletTexture,
                       TextFormat("AMMO: %d / %d", currentGun->currentAmmo, currentGun->reserveAmmo));
        if (player->isReloading) {
            DrawText("RELOAD", (int)iconStartX + 200, (int)(lineY0 + lineGap * 2.0f), 22, YELLOW);
        }
    } else if (currentGun->type == GUN_KNIFE) {
        DrawHudIconRow((Vector2){iconStartX, lineY0 + lineGap * 2.0f},
                       hudBulletTexture,
                       "Inf");
    }

    // Row 4: Masks visualization
    float maskY = lineY0 + lineGap * 3.5f;
    DrawText("MASKS [4-6]:", (int)iconStartX, (int)maskY, 20, LIGHTGRAY);
    for (int i=0; i<3; i++) {
        Color slotColor = DARKGRAY;
        if (inv->maskSlots[i].collected) {
            slotColor = inv->maskSlots[i].color; 
            if (i == inv->currentMaskIndex) slotColor = ColorBrightness(slotColor, 0.3f); // Highlight selected
        }
        
        // Draw slot box
        float slotSize = 30.0f;
        float slotX = iconStartX + 120 + i * (slotSize + 10);
        DrawRectangle((int)slotX, (int)maskY, (int)slotSize, (int)slotSize, slotColor);
        DrawRectangleLines((int)slotX, (int)maskY, (int)slotSize, (int)slotSize, WHITE);

        if (i == inv->currentMaskIndex) {
            DrawRectangleLinesEx((Rectangle){slotX-2, maskY-2, slotSize+4, slotSize+4}, 2.0f, GOLD);
        }

        // Timer bar
        if (inv->maskSlots[i].isActive) {
            float pct = inv->maskSlots[i].currentTimer / inv->maskSlots[i].maxDuration;
            DrawRectangle((int)slotX, (int)(maskY + slotSize + 2), (int)(slotSize * pct), 5, GREEN);
        }
    }
}
