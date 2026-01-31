#include "game.h"

#include <math.h>
#include <stdio.h>

#include "../raylib/src/raylib.h"
#include "../raylib/src/raymath.h"
#include "anim.h"
#include "enemies/enemy.h"
#include "entity.h"
#include "episodes/episodes.h"
#include "levels.h"
#include "player/player.h"
#include "player/player_render.h"
#include "types.h"

#include "ui/hud.h"

// Game Constants
#define MAX_BULLETS 100
#define BULLET_SPEED 800.0f
#define BULLET_RADIUS 5.0f
#define BULLET_LIFETIME 2.0f

// Ammo constants
#define MAG_SIZE 12
#define RESERVE_AMMO_START 48
#define RELOAD_DURATION 1.5f  // Duration of reload animation in seconds

// --- GAME STATE ---
typedef enum {
    STATE_MENU,
    STATE_PLAYING,
    STATE_GAMEOVER,
    STATE_WIN
} GameState;

// --- GAME CONTEXT ---
// This holds progression state that should be shared across screens and later used for saving/loading.
typedef struct {
    int nextEpisodeId;     // The episode a player should play next (story progression).
    bool hasProgress;      // True once the player has started at least one episode.
    bool hasWonLastEpisode; // Set when the active episode is completed.

    // Player progression context (carry to the next episode).
    // This is the core of future save/load.
    struct {
        bool valid;
        Identity identity;
        PlayerEquipState equipped;
        int magAmmo;
        int reserveAmmo;

        // Lightweight inventory (which weapons are owned/unlocked).
        // We treat knife + bare hands as always available.
        bool hasFlashlight;
        bool hasHandgun;
        bool hasRifle;
        bool hasShotgun;
    } player;
} GameContext;

static GameState currentState;
static Level currentLevel;
static bool gameOver; 
static bool gameWon; 

static GameContext gameCtx;

static void SavePlayerContextFromLivePlayer(const Entity *livePlayer) {
    if (!livePlayer) return;
    gameCtx.player.valid = true;
    gameCtx.player.identity = livePlayer->identity;
    gameCtx.player.equipped = livePlayer->equipmentState;
    gameCtx.player.magAmmo = livePlayer->magAmmo;
    gameCtx.player.reserveAmmo = livePlayer->reserveAmmo;

    // Inventory inference: if you've ever equipped it, you own it.
    // (Later, real pickups can set these flags directly.)
    switch (livePlayer->equipmentState) {
        case PLAYER_EQUIP_FLASHLIGHT: gameCtx.player.hasFlashlight = true; break;
        case PLAYER_EQUIP_HANDGUN: gameCtx.player.hasHandgun = true; break;
        case PLAYER_EQUIP_RIFLE: gameCtx.player.hasRifle = true; break;
        case PLAYER_EQUIP_SHOTGUN: gameCtx.player.hasShotgun = true; break;
        default: break;
    }
}

static bool PlayerContext_AllowsEquip(PlayerEquipState equip) {
    // Always allowed
    if (equip == PLAYER_EQUIP_BARE_HANDS || equip == PLAYER_EQUIP_KNIFE) return true;

    if (!gameCtx.player.valid) return false;

    switch (equip) {
        case PLAYER_EQUIP_FLASHLIGHT: return gameCtx.player.hasFlashlight;
        case PLAYER_EQUIP_HANDGUN: return gameCtx.player.hasHandgun;
        case PLAYER_EQUIP_RIFLE: return gameCtx.player.hasRifle;
        case PLAYER_EQUIP_SHOTGUN: return gameCtx.player.hasShotgun;
        default: return false;
    }
}

static Entity player;
static Camera2D camera;

static Bullet bullets[MAX_BULLETS];
static Entity droppedMask;
static bool maskActive;

static PlayerRender playerRender;
static const float weaponShootHold = 0.2f;
static float weaponShootTimer = 0.0f;
static const float meleeRange = 80.0f;
static const float meleePromptOffset = 25.0f;
static const float meleePromptHorizontalOffset = 40.0f;
static const float droppedMaskRadius = 15.0f;
static const float playerDebugCrossSize = 6.0f;
static const bool playerDebugDraw = false;
static int meleeTargetIndex = -1;
static Vector2 playerFramePivot = {0.1f, 0.1f};
static bool playerFramePivotReady = false;
static const unsigned char playerPivotAlphaThreshold = 32;

// Track equipment transitions for gameplay (rendering handles its own internal state).
static PlayerEquipState lastEquipmentState;

// HUD is implemented in ui/hud.c now.

static void DrawPlayerFallback(Vector2 position, float radius) {
    float size = radius * 2.0f;
    DrawRectangleV((Vector2){position.x - radius, position.y - radius},
                   (Vector2){size, size}, RED);
}

static void UpdatePlayerPivotFromFrame(Texture2D frame) {
    if (playerFramePivotReady || frame.id == 0) {
        return;
    }
    Image image = LoadImageFromTexture(frame);
    if (image.data == NULL) {
        TraceLog(LOG_WARNING, "Failed to read player frame for pivot");
        return;
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
            if (alpha > playerPivotAlphaThreshold) {
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
        playerFramePivot = (Vector2){centerX / (float)image.width,
                                     centerY / (float)image.height};
        playerFramePivotReady = true;
    } else {
        TraceLog(LOG_WARNING, "Player frame had no opaque pixels for pivot");
    }
    UnloadImage(image);
}

static int GetClosestEnemyInRange(Vector2 position, float range) {
    float closestDist = range;
    int closestIndex = -1;
    for (int i = 0; i < currentLevel.enemyCount; i++) {
        if (!currentLevel.enemies[i].active) {
            continue;
        }
        float dist = Vector2Distance(position, currentLevel.enemies[i].position);
        if (dist <= closestDist) {
            closestDist = dist;
            closestIndex = i;
        }
    }
    return closestIndex;
}

static void HandleEnemyKilled(int enemyIndex) {
    if (enemyIndex < 0 || enemyIndex >= currentLevel.enemyCount) {
        return;
    }
    if (!currentLevel.enemies[enemyIndex].active) {
        return;
    }
    currentLevel.enemies[enemyIndex].active = false;
    maskActive = true;
    droppedMask.identity = currentLevel.enemies[enemyIndex].identity;
    droppedMask.position = currentLevel.enemies[enemyIndex].position;
    droppedMask.radius = droppedMaskRadius;
    droppedMask.active = true;
    // Auto-equip handgun after a kill (so player can shoot instead of staying on knife).
    // If you later add actual weapon pickups, move this to that pickup logic instead.
    if (player.equipmentState == PLAYER_EQUIP_KNIFE || player.equipmentState == PLAYER_EQUIP_BARE_HANDS) {
        player.equipmentState = PLAYER_EQUIP_HANDGUN;
        TraceLog(LOG_INFO, "Player auto-equipped handgun after kill");
    }
}

// --- MENU BUTTONS ---
static Rectangle btnEpisode1;
static Rectangle btnEpisode2;
static Rectangle btnQuit;

// Player menu (shown in non-dev builds)
static Rectangle btnPlay;
static Rectangle btnContinue;

// Helper to reset game state for a specific level
static void StartLevel(int episodeId) {
    gameOver = false;
    gameWon = false;
    gameCtx.hasWonLastEpisode = false;
    maskActive = false;
    currentState = STATE_PLAYING;

    // Reset bullets
    for (int i = 0; i < MAX_BULLETS; i++) bullets[i].active = false;

    // Init Level
    InitLevel(episodeId, &currentLevel);

    // Progress context
    gameCtx.hasProgress = true;

    // Player
    player = InitPlayer(currentLevel.playerSpawn, currentLevel.playerStartId);
    player.rotation = 0.0f;
    player.isReloading = false;
    player.reloadTimer = 0.0f;
    weaponShootTimer = 0.0f;

    // Story progression: if we have a saved player context AND we're starting the next episode,
    // carry identity/equipment/ammo/inventory across.
    bool shouldCarryPlayer = gameCtx.player.valid && (episodeId == gameCtx.nextEpisodeId);
    if (shouldCarryPlayer) {
        player.identity = gameCtx.player.identity;
        player.magAmmo = gameCtx.player.magAmmo;
        player.reserveAmmo = gameCtx.player.reserveAmmo;
        player.equipmentState = PlayerContext_AllowsEquip(gameCtx.player.equipped)
                                  ? gameCtx.player.equipped
                                  : PLAYER_EQUIP_KNIFE;
    } else {
        // Fresh episode start (new game / replay): defaults
        player.equipmentState = PLAYER_EQUIP_KNIFE;
        player.magAmmo = MAG_SIZE;
        player.reserveAmmo = RESERVE_AMMO_START;

        // Also reset inventory to a minimal baseline.
    gameCtx.player.valid = true;
    gameCtx.player.identity = player.identity;
    gameCtx.player.equipped = player.equipmentState;
    gameCtx.player.magAmmo = player.magAmmo;
    gameCtx.player.reserveAmmo = player.reserveAmmo;
    gameCtx.player.hasFlashlight = true; // optional baseline; feels nice for early game
    gameCtx.player.hasHandgun = false;
    gameCtx.player.hasRifle = false;
    gameCtx.player.hasShotgun = false;
    }
    lastEquipmentState = player.equipmentState;

    // Camera
    camera = (Camera2D){0};
    camera.target = player.position;
    camera.offset = (Vector2){ GetScreenWidth() / 2.0f, GetScreenHeight() / 2.0f };
    camera.rotation = 0.0f;
    camera.zoom = 1.0f;

    // Player visuals/animations
    PlayerRender_Init(&playerRender);
    playerRender.lastEquip = player.equipmentState;
    PlayerRender_LoadEpisodeAssets(&playerRender);
    
    playerFramePivotReady = false;
    if (playerRender.loaded) {
        UpdatePlayerPivotFromFrame(AnimPlayer_GetFrame(&playerRender.feetAnim));
    }
}

void Game_Init(void) {
    currentState = STATE_MENU;

    // Start a fresh story by default.
    gameCtx = (GameContext){0};
    gameCtx.nextEpisodeId = 1;

    // Default baseline player context (future: load-from-save).
    gameCtx.player.valid = false;

    Hud_Init();

    // Define Menu Buttons (Centered)
    int sw = GetScreenWidth();
    int sh = GetScreenHeight();
    int btnWidth = 200;
    int btnHeight = 50;
    int startY = sh / 2 - 50;

    btnEpisode1 = (Rectangle){ (float)sw/2.0f - (float)btnWidth/2.0f, (float)startY, (float)btnWidth, (float)btnHeight };
    btnEpisode2 = (Rectangle){ (float)sw/2.0f - (float)btnWidth/2.0f, (float)startY + 70, (float)btnWidth, (float)btnHeight };
    btnQuit     = (Rectangle){ (float)sw/2.0f - (float)btnWidth/2.0f, (float)startY + 140, (float)btnWidth, (float)btnHeight };

    // Player-facing menu buttons reuse the same layout slots.
    btnPlay     = btnEpisode1;
    btnContinue = btnEpisode2;
}

// Helper for HighDPI Mouse Scaling
static Vector2 GetScaledMousePosition(void) {
    Vector2 mousePos = GetMousePosition();
    float scaleX = (float)GetScreenWidth() / (float)GetRenderWidth();
    float scaleY = (float)GetScreenHeight() / (float)GetRenderHeight();

    if (GetScreenWidth() > 0 && GetRenderWidth() > 0) {
        mousePos.x *= scaleX;
        mousePos.y *= scaleY;
    }
    return mousePos;
}

static void UpdateMenu(void) {
    Vector2 mousePos = GetScaledMousePosition();

    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        // Developer build: manual episode picking.
#if defined(DEV_MODE) && (DEV_MODE)
        if (CheckCollisionPointRec(mousePos, btnEpisode1)) {
            StartLevel(1);
        } else if (CheckCollisionPointRec(mousePos, btnEpisode2)) {
            StartLevel(2);
        } else if (CheckCollisionPointRec(mousePos, btnQuit)) {
            CloseWindow();
        }
#else
        // Player build: story-driven flow.
        if (CheckCollisionPointRec(mousePos, btnPlay)) {
            // New Game
            gameCtx.nextEpisodeId = 1;
            StartLevel(gameCtx.nextEpisodeId);
        } else if (CheckCollisionPointRec(mousePos, btnContinue)) {
            // Continue (only if player has started before)
            if (gameCtx.hasProgress) {
                StartLevel(gameCtx.nextEpisodeId);
            }
        } else if (CheckCollisionPointRec(mousePos, btnQuit)) {
            CloseWindow();
        }
#endif
    }
}

static void DrawMenu(void) {
    BeginDrawing();
    ClearBackground((Color){20, 20, 25, 255});

    int sw = GetScreenWidth();
    int btnWidth = 200;

    DrawText("GGJ26 - MASK INFILTRATION", sw/2 - 200, 100, 30, RAYWHITE);

    // Buttons
    Color hoverColor = (Color){50, 50, 60, 255};
    Color normalColor = GRAY;
    Vector2 mousePos = GetScaledMousePosition();

    // Developer build: manual stage/episode picking.
#if defined(DEV_MODE) && (DEV_MODE)
    DrawRectangleRec(btnEpisode1, CheckCollisionPointRec(mousePos, btnEpisode1) ? hoverColor : normalColor);
    DrawText("EPISODE 1", btnEpisode1.x + 40, btnEpisode1.y + 15, 20, WHITE);

    DrawRectangleRec(btnEpisode2, CheckCollisionPointRec(mousePos, btnEpisode2) ? hoverColor : normalColor);
    DrawText("EPISODE 2", btnEpisode2.x + 40, btnEpisode2.y + 15, 20, WHITE);
#else
    // Player build: story menu.
    DrawRectangleRec(btnPlay, CheckCollisionPointRec(mousePos, btnPlay) ? hoverColor : normalColor);
    DrawText("PLAY", btnPlay.x + 70, btnPlay.y + 15, 20, WHITE);

    bool canContinue = gameCtx.hasProgress;
    Color continueColor = canContinue ? normalColor : Fade(normalColor, 0.35f);
    Color continueTextColor = canContinue ? WHITE : Fade(WHITE, 0.5f);
    DrawRectangleRec(btnContinue, CheckCollisionPointRec(mousePos, btnContinue) && canContinue ? hoverColor : continueColor);
    DrawText("CONTINUE", btnContinue.x + 45, btnContinue.y + 15, 20, continueTextColor);
#endif

    // Quit
    DrawRectangleRec(btnQuit, CheckCollisionPointRec(mousePos, btnQuit) ? hoverColor : normalColor);
    DrawText("QUIT", btnQuit.x + 75, btnQuit.y + 15, 20, WHITE);
    
    EndDrawing();
}

static void UpdateGame(float dt) {
    // Update Camera Offset
    camera.offset = (Vector2){ GetScreenWidth() / 2.0f, GetScreenHeight() / 2.0f };

    // Game Over / Win Logic Inputs
    if (gameOver || gameWon) {
        // GAME OVER
        if (gameOver) {
            if (IsKeyPressed(KEY_R)) {
                StartLevel(currentLevel.id); // Restart current level
            }
            if (IsKeyPressed(KEY_M) || IsKeyPressed(KEY_SPACE)) {
                currentState = STATE_MENU; // Return to menu
            }
            return;
        }

        // WIN
        if (gameWon) {
            // Update progress context (used by Continue / Next).
            gameCtx.hasWonLastEpisode = true;
            SavePlayerContextFromLivePlayer(&player);
            if (currentLevel.id == 1) {
                gameCtx.nextEpisodeId = 2;
            }

            // Player builds: Next or Menu.
#if !defined(DEV_MODE) || !(DEV_MODE)
            if (IsKeyPressed(KEY_N)) {
                StartLevel(gameCtx.nextEpisodeId);
                return;
            }
            if (IsKeyPressed(KEY_M) || IsKeyPressed(KEY_SPACE)) {
                currentState = STATE_MENU;
                return;
            }
#else
            // Dev builds: Replay, Next, or Menu.
            if (IsKeyPressed(KEY_R)) {
                StartLevel(currentLevel.id);
                return;
            }
            if (IsKeyPressed(KEY_N)) {
                StartLevel(gameCtx.nextEpisodeId);
                return;
            }
            if (IsKeyPressed(KEY_M) || IsKeyPressed(KEY_SPACE)) {
                currentState = STATE_MENU;
                return;
            }
#endif
        }
        return;
    }
    
    // Helper function to check if player has a gun equipped
    bool hasGunEquipped = (player.equipmentState == PLAYER_EQUIP_HANDGUN ||
                          player.equipmentState == PLAYER_EQUIP_RIFLE ||
                          player.equipmentState == PLAYER_EQUIP_SHOTGUN);

    // Player Update
    UpdatePlayer(&player, &currentLevel, dt);

    // Detect equipment changes
    if (player.equipmentState != lastEquipmentState) {
        weaponShootTimer = 0.0f;
    PlayerRender_OnEquip(&playerRender, player.equipmentState);
        lastEquipmentState = player.equipmentState;
    }

    // Camera Update
    camera.target = Vector2Lerp(camera.target, player.position, 0.1f);

    // Player Aiming
    Vector2 mouseWorld = GetScreenToWorld2D(GetMousePosition(), camera);
    Vector2 aimDir = Vector2Subtract(mouseWorld, player.position);
    Vector2 aimDirNormalized =
        Vector2Length(aimDir) > 0.01f ? Vector2Normalize(aimDir) : (Vector2){1.0f, 0.0f};

    // 0 degrees from atan2 is pointing RIGHT (+X).
    // Set this offset to match how the sprite is authored.
    // If your sprite faces UP by default, start with -90
    const float spriteFacingOffsetDeg = 0.0f;

    // If rotation is mirrored (turns the wrong way), flip sign:
    // float aimDeg = -atan2f(aimDirNormalized.y, aimDirNormalized.x) * RAD2DEG;
    float aimDeg = atan2f(aimDirNormalized.y, aimDirNormalized.x) * RAD2DEG;

    player.rotation = aimDeg + spriteFacingOffsetDeg;
    if (weaponShootTimer > 0.0f) {
        weaponShootTimer -= dt;
        if (weaponShootTimer < 0.0f) {
            weaponShootTimer = 0.0f;
        }
    }

    // Update all player visual animation state in one place.
    PlayerRender_Update(&playerRender, &player, dt, weaponShootTimer);
    
    // Handle reload timer
    if (player.isReloading) {
        player.reloadTimer -= dt;
        if (player.reloadTimer <= 0.0f) {
            // Reload complete - transfer ammo
            int ammoNeeded = MAG_SIZE - player.magAmmo;
            int ammoToTransfer = ammoNeeded < player.reserveAmmo ? ammoNeeded : player.reserveAmmo;
            player.magAmmo += ammoToTransfer;
            player.reserveAmmo -= ammoToTransfer;
            player.isReloading = false;
            player.reloadTimer = 0.0f;
        }
    }

    // Reload input (R key)
    if (IsKeyPressed(KEY_R) && !player.isReloading) {
        bool canReload = hasGunEquipped && player.magAmmo < MAG_SIZE && player.reserveAmmo > 0;
        if (canReload) {
            player.isReloading = true;
            player.reloadTimer = RELOAD_DURATION;
        }
    }

    // Player Shooting
    bool shootPressed = IsMouseButtonPressed(MOUSE_LEFT_BUTTON) || IsKeyPressed(KEY_SPACE);
    if (shootPressed) {
        bool canShoot = hasGunEquipped && HasAbility(player.identity, ABILITY_SHOOT) && !player.isReloading;
        
        if (canShoot) {
            if (player.magAmmo > 0) {
                // Shoot
                for (int i = 0; i < MAX_BULLETS; i++) {
                    if (!bullets[i].active) {
                        bullets[i].active = true;
                        bullets[i].position = player.position;
                        bullets[i].radius = BULLET_RADIUS;
                        bullets[i].lifeTime = BULLET_LIFETIME;
                        bullets[i].isPlayerOwned = true;
                        bullets[i].velocity = Vector2Scale(aimDirNormalized, BULLET_SPEED);
                        weaponShootTimer = weaponShootHold;
                        player.magAmmo--;
                        break;
                    }
                }
            } else {
                // Auto-reload when trying to shoot with empty mag
                if (player.reserveAmmo > 0) {
                    player.isReloading = true;
                    player.reloadTimer = RELOAD_DURATION;
                }
            }
        }
    }

    meleeTargetIndex = -1;
    if (player.equipmentState == PLAYER_EQUIP_KNIFE || 
        player.equipmentState == PLAYER_EQUIP_FLASHLIGHT ||
        player.equipmentState == PLAYER_EQUIP_BARE_HANDS) {
        meleeTargetIndex = GetClosestEnemyInRange(player.position, meleeRange);
        if (meleeTargetIndex != -1 && IsKeyPressed(KEY_E)) {
            HandleEnemyKilled(meleeTargetIndex);
            meleeTargetIndex = -1;
        }
    }

    // Enemy Update
    for (int i = 0; i < currentLevel.enemyCount; i++) {
        UpdateEnemy(&currentLevel.enemies[i], player.position, bullets, MAX_BULLETS, dt);
    }

    // Bullet Updates & Collisions
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (!bullets[i].active) continue;

        bullets[i].position = Vector2Add(bullets[i].position, Vector2Scale(bullets[i].velocity, dt));
        bullets[i].lifeTime -= dt;

        // Check Wall Collisions
        for (int w = 0; w < currentLevel.wallCount; w++) {
            if (CheckCollisionCircleRec(bullets[i].position, bullets[i].radius, currentLevel.walls[w])) {
                bullets[i].active = false;
            }
        }
        // Check Door Collisions
        for (int d = 0; d < currentLevel.doorCount; d++) {
            if (CheckCollisionCircleRec(bullets[i].position, bullets[i].radius, currentLevel.doors[d])) {
                bullets[i].active = false;
            }
        }
        if (!bullets[i].active) continue;

        if (bullets[i].isPlayerOwned) {
            // Check against Enemies
            for (int e = 0; e < currentLevel.enemyCount; e++) {
                if (currentLevel.enemies[e].active &&
                    CheckCollisionCircles(bullets[i].position, bullets[i].radius,
                                          currentLevel.enemies[e].position, currentLevel.enemies[e].radius)) {
                    bullets[i].active = false;
                    HandleEnemyKilled(e);
                }
            }
        } else {
            // Check against Player
            if (CheckCollisionCircles(bullets[i].position, bullets[i].radius, player.position, player.radius)) {
                gameOver = true;
                bullets[i].active = false;
            }
        }
    }

    // Mask Pickup
    if (maskActive) {
        if (CheckCollisionCircles(player.position, player.radius, droppedMask.position, droppedMask.radius)) {
            if (IsKeyPressed(KEY_SPACE)) {
                player.identity = droppedMask.identity;
                maskActive = false;
            }
        }
    }

    // Door State Logic
    for (int i = 0; i < currentLevel.doorCount; i++) {
        if (CheckCollisionCircleRec(player.position, player.radius, currentLevel.doors[i]) &&
            player.identity.permissionLevel >= currentLevel.doorPerms[i]) {
            currentLevel.doorsOpen[i] = true;
        } else {
            currentLevel.doorsOpen[i] = false;
        }
    }

    // Win Condition
    if (player.position.x > currentLevel.winX) {
        gameWon = true; // Win current level
    }
}

static void DrawGame(void) {
    BeginDrawing();
    ClearBackground((Color){20, 20, 25, 255});

    if (gameOver || gameWon) {
        int sw = GetScreenWidth();
        int sh = GetScreenHeight();

        if (gameOver) {
            DrawText("GAME OVER", sw / 2 - 100, sh / 2 - 20, 40, RED);
            DrawText("Press R to Restart Level", sw / 2 - 100, sh / 2 + 30, 20, RAYWHITE);
            DrawText("Press M to Menu", sw / 2 - 100, sh / 2 + 60, 20, RAYWHITE);
        } else {
            DrawText("EPISODE COMPLETE!", sw / 2 - 150, sh / 2 - 20, 40, GOLD);
            // Player builds: Next + Menu.
#if !defined(DEV_MODE) || !(DEV_MODE)
            DrawText("Press N to Next", sw / 2 - 100, sh / 2 + 30, 20, RAYWHITE);
            DrawText("Press M to Menu", sw / 2 - 100, sh / 2 + 60, 20, RAYWHITE);
            DrawText("(Saving will be added later)", sw / 2 - 130, sh / 2 + 90, 18, Fade(RAYWHITE, 0.7f));
#else
            // Dev builds: Replay still useful.
            DrawText("Press R to Replay", sw / 2 - 100, sh / 2 + 30, 20, RAYWHITE);
            DrawText("Press N to Next", sw / 2 - 100, sh / 2 + 60, 20, RAYWHITE);
            DrawText("Press M to Menu", sw / 2 - 100, sh / 2 + 90, 20, RAYWHITE);
#endif
        }
    } else {
        BeginMode2D(camera);
        // Draw Level Elements
        for (int i = 0; i < currentLevel.wallCount; i++)
            DrawRectangleRec(currentLevel.walls[i], GRAY);
        for (int i = 0; i < currentLevel.doorCount; i++) {
            Color dColor = currentLevel.doorsOpen[i] ? GREEN : DARKGRAY;
            if (!currentLevel.doorsOpen[i]) {
                DrawRectangleRec(currentLevel.doors[i], dColor);
                DrawText(TextFormat("REQ: LVL %d", currentLevel.doorPerms[i]),
                         (int)currentLevel.doors[i].x - 10,
                         (int)currentLevel.doors[i].y + 50, 10, WHITE);
            } else {
                DrawRectangleLinesEx(currentLevel.doors[i], 3.0f, GREEN);
            }
        }

        if (currentLevel.id == 1) {
            DrawText("ZONE 1: STAFF ONLY", 400, 300, 30, Fade(WHITE, 0.1f));
            DrawText("ZONE 2: GUARD ONLY", 1400, 300, 30, Fade(WHITE, 0.1f));
            DrawText("ZONE 3: ADMIN ONLY", 2400, 300, 30, Fade(WHITE, 0.1f));
            DrawText("EXIT", 3200, 300, 40, Fade(GREEN, 0.5f));
        }

        for (int i = 0; i < currentLevel.enemyCount; i++) {
            if (currentLevel.enemies[i].active) {
                DrawCircleV(currentLevel.enemies[i].position,
                            currentLevel.enemies[i].radius,
                            currentLevel.enemies[i].identity.color);
                DrawCircleLines((int)currentLevel.enemies[i].position.x,
                                (int)currentLevel.enemies[i].position.y,
                                currentLevel.enemies[i].radius + 2, WHITE);
            }
        }

        if (maskActive) {
            DrawCircleV(droppedMask.position, droppedMask.radius,
                        droppedMask.identity.color);
            DrawCircleLines((int)droppedMask.position.x, (int)droppedMask.position.y,
                            droppedMask.radius + 4, GOLD);
            DrawText("PRESS SPACE", (int)droppedMask.position.x - 30,
                     (int)droppedMask.position.y - 30, 10, WHITE);
        }

        for (int i = 0; i < MAX_BULLETS; i++) {
            if (bullets[i].active) {
                DrawCircleV(bullets[i].position, bullets[i].radius,
                            bullets[i].isPlayerOwned ? YELLOW : ORANGE);
            }
        }

        if (playerRender.loaded) {
            PlayerRender_Draw(&playerRender, &player);
            PlayerRender_DrawMuzzleFlash(&playerRender, &player, weaponShootTimer);
        } else {
            DrawPlayerFallback(player.position, player.radius);
        }
        if (meleeTargetIndex >= 0 && meleeTargetIndex < currentLevel.enemyCount &&
            currentLevel.enemies[meleeTargetIndex].active) {
            Vector2 promptPos = {
                currentLevel.enemies[meleeTargetIndex].position.x - meleePromptHorizontalOffset,
                currentLevel.enemies[meleeTargetIndex].position.y - meleePromptOffset
            };
            DrawText("PRESS E TO CHOKE", (int)promptPos.x, (int)promptPos.y, 12, WHITE);
        }
        if (playerDebugDraw) {
            DrawLineV((Vector2){player.position.x - playerDebugCrossSize, player.position.y},
                      (Vector2){player.position.x + playerDebugCrossSize, player.position.y}, RED);
            DrawLineV((Vector2){player.position.x, player.position.y - playerDebugCrossSize},
                      (Vector2){player.position.x, player.position.y + playerDebugCrossSize}, RED);
            DrawCircleLines((int)player.position.x, (int)player.position.y, player.radius, GOLD);
        }
        Vector2 mouseWorld = GetScreenToWorld2D(GetMousePosition(), camera);
        DrawLineV(player.position, mouseWorld, Fade(WHITE, 0.2f));
        EndMode2D();

    // HUD
    Hud_DrawPlayer(&player);
    }

    EndDrawing();
}

void Game_Update(void) {
    if (currentState == STATE_MENU) {
        UpdateMenu();
    } else {
        UpdateGame(GetFrameTime());
    }
}

void Game_Draw(void) {
    if (currentState == STATE_MENU) {
        DrawMenu();
    } else {
        DrawGame();
    }
}


void Game_Shutdown(void) {
    PlayerRender_Unload(&playerRender);
    Hud_Shutdown();
}

