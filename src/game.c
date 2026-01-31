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
#include "player.h"
#include "types.h"

// Game Constants
#define MAX_BULLETS 100
#define BULLET_SPEED 800.0f
#define BULLET_RADIUS 5.0f
#define BULLET_LIFETIME 2.0f

// Ammo Constants
#define MAG_SIZE 12
#define INITIAL_RESERVE_AMMO 48

// Muzzle Flash Offset Constants
#define MUZZLE_OFFSET_X 40.0f
#define MUZZLE_OFFSET_Y -10.0f

// --- GAME STATE ---
typedef enum {
    STATE_MENU,
    STATE_PLAYING,
    STATE_GAMEOVER,
    STATE_WIN
} GameState;

static GameState currentState;
static Level currentLevel;
static bool gameOver; 
static bool gameWon; 

static Entity player;
static Camera2D camera;

static Bullet bullets[MAX_BULLETS];
static Entity droppedMask;
static bool maskActive;

typedef enum {
    PLAYER_ANIM_IDLE,
    PLAYER_ANIM_WALK
} PlayerAnimState;

typedef enum {
    PLAYER_GUN_EQUIP,
    PLAYER_GUN_IDLE,
    PLAYER_GUN_WALK,
    PLAYER_GUN_SHOOT,
    PLAYER_GUN_RELOAD
} PlayerGunAnimState;

static AnimClip playerIdleClip;
static AnimClip playerWalkClip;
static AnimClip playerGunIdleClip;
static AnimClip playerGunWalkClip;
static AnimClip playerGunShootClip;
static AnimClip playerGunReloadClip;
static AnimPlayer playerAnim;
static AnimPlayer playerGunAnim;
static Texture2D playerShadow;
static Texture2D muzzleFlashTexture;
static bool playerAnimLoaded;
static bool playerGunAnimLoaded;
static bool playerGunReloadAnimLoaded;
static bool muzzleFlashLoaded;
static PlayerAnimState playerAnimState;
static PlayerGunAnimState playerGunAnimState;
static const float playerSpriteScale = 0.3f;
static const Vector2 playerSpritePivot = {0.5f, 0.5f};
static const float playerGunShootHold = 0.2f;
static float playerGunShootTimer = 0.0f;
static const float muzzleFlashDuration = 0.05f;  // How long the muzzle flash is visible
static float muzzleFlashTimer = 0.0f;
static const float reloadTransferFrame = 0.7f;  // At 70% through reload, transfer ammo
static bool reloadAmmoTransferred = false;
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

// Track equipment transitions to play one-shot equip animation.
static PlayerEquipState lastEquipmentState;

static void DrawPlayerFallback(Vector2 position, float radius) {
    float size = radius * 2.0f;
    DrawRectangleV((Vector2){position.x - radius, position.y - radius},
                   (Vector2){size, size}, RED);
}


static void DrawPlayerFrame(Texture2D frame, Vector2 position, float rotation) {
    Rectangle source = (Rectangle){0.0f, 0.0f, (float)frame.width, (float)frame.height};

    float destW = frame.width * playerSpriteScale;
    float destH = frame.height * playerSpriteScale;

    // Destination rectangle is centered at position
    Rectangle dest = (Rectangle){ position.x, position.y, destW, destH };

    // Origin is the pivot within the destination rect (half-size for centering)
    Vector2 origin = (Vector2){ destW * playerSpritePivot.x, destH * playerSpritePivot.y };

    DrawTexturePro(frame, source, dest, origin, rotation, WHITE);
}

static void DrawPlayerShadow(Texture2D shadow, Vector2 position) {
    Rectangle source = (Rectangle){0.0f, 0.0f, (float)shadow.width, (float)shadow.height};

    float destW = shadow.width * playerSpriteScale;
    float destH = shadow.height * playerSpriteScale;

    // Destination rectangle is centered at position (optionally offset slightly down if desired)
    Rectangle dest = (Rectangle){ position.x, position.y, destW, destH };

    // Origin centered; no rotation for shadow
    Vector2 origin = (Vector2){ destW * playerSpritePivot.x, destH * playerSpritePivot.y };

    DrawTexturePro(shadow, source, dest, origin, 0.0f, WHITE);
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
    if (player.equipmentState == PLAYER_EQUIP_BARE_HANDS) {
        player.equipmentState = PLAYER_EQUIP_GUN;
        TraceLog(LOG_INFO, "Player granted gun after first kill");
    }
}

// --- MENU BUTTONS ---
static Rectangle btnEpisode1;
static Rectangle btnEpisode2;
static Rectangle btnQuit;

// Helper to reset game state for a specific level
static void StartLevel(int episodeId) {
    gameOver = false;
    gameWon = false;
    maskActive = false;
    currentState = STATE_PLAYING;

    // Reset bullets
    for (int i = 0; i < MAX_BULLETS; i++) bullets[i].active = false;

    // Init Level
    InitLevel(episodeId, &currentLevel);

    // Player
    player = InitPlayer(currentLevel.playerSpawn, currentLevel.playerStartId);
    player.equipmentState = PLAYER_EQUIP_BARE_HANDS;
    player.rotation = 0.0f;
    player.magAmmo = MAG_SIZE;
    player.reserveAmmo = INITIAL_RESERVE_AMMO;
    playerGunShootTimer = 0.0f;
    muzzleFlashTimer = 0.0f;
    reloadAmmoTransferred = false;
    lastEquipmentState = player.equipmentState;

    // Camera
    camera = (Camera2D){0};
    camera.target = player.position;
    camera.offset = (Vector2){ GetScreenWidth() / 2.0f, GetScreenHeight() / 2.0f };
    camera.rotation = 0.0f;
    camera.zoom = 1.0f;

    if (playerAnimLoaded) {
        UnloadAnimClip(&playerIdleClip);
        UnloadAnimClip(&playerWalkClip);
        if (playerShadow.id != 0) {
            UnloadTexture(playerShadow);
            playerShadow = (Texture2D){0};
        }
        playerAnimLoaded = false;
    }
    if (playerGunAnimLoaded) {
        UnloadAnimClip(&playerGunIdleClip);
        UnloadAnimClip(&playerGunWalkClip);
        UnloadAnimClip(&playerGunShootClip);
        playerGunAnimLoaded = false;
    }
    if (playerGunReloadAnimLoaded) {
        UnloadAnimClip(&playerGunReloadClip);
        playerGunReloadAnimLoaded = false;
    }
    if (muzzleFlashLoaded) {
        UnloadTexture(muzzleFlashTexture);
        muzzleFlashTexture = (Texture2D){0};
        muzzleFlashLoaded = false;
    }
    playerIdleClip = LoadAnimClip("assets/character/LightArtilleryRobot/idle60", 30.0f);
    playerWalkClip = LoadAnimClip("assets/character/LightArtilleryRobot/walk60", 60.0f);
    playerGunIdleClip = LoadAnimClip("assets/character/LightArtilleryRobot/idle60", 30.0f);
    playerGunWalkClip = LoadAnimClip("assets/character/LightArtilleryRobot/walk60", 60.0f);
    playerGunShootClip = LoadAnimClip("assets/character/LightArtilleryRobot/shootGun20", 60.0f);
    playerGunReloadClip = LoadAnimClip("assets/character/LightArtilleryRobot/idle60", 30.0f);
    playerShadow = LoadTexture("assets/character/LightArtilleryRobot/shadow.png");
    muzzleFlashTexture = LoadTexture("assets/effects/muzzle_flash_01.tga");
    playerAnimLoaded = playerIdleClip.frame_count > 0 && playerWalkClip.frame_count > 0 &&
                       playerShadow.id != 0;
    playerGunAnimLoaded = playerGunIdleClip.frame_count > 0 &&
                          playerGunWalkClip.frame_count > 0 &&
                          playerGunShootClip.frame_count > 0;
    playerGunReloadAnimLoaded = playerGunReloadClip.frame_count > 0;
    muzzleFlashLoaded = muzzleFlashTexture.id != 0;
    playerAnim = (AnimPlayer){0};
    playerGunAnim = (AnimPlayer){0};
    playerAnimState = PLAYER_ANIM_IDLE;
    playerGunAnimState = PLAYER_GUN_IDLE;
    AnimPlayer_SetClip(&playerAnim, &playerIdleClip);
    if (playerGunAnimLoaded) {
        AnimPlayer_SetClip(&playerGunAnim, &playerGunIdleClip);
    }
    playerFramePivotReady = false;
    if (playerAnimLoaded) {
        UpdatePlayerPivotFromFrame(AnimPlayer_GetFrame(&playerAnim));
    }
    if (!playerAnimLoaded) {
        TraceLog(LOG_ERROR, "Player animation assets missing, using fallback");
    }
}

void Game_Init(void) {
    currentState = STATE_MENU;

    // Define Menu Buttons (Centered)
    int sw = GetScreenWidth();
    int sh = GetScreenHeight();
    int btnWidth = 200;
    int btnHeight = 50;
    int startY = sh / 2 - 50;

    btnEpisode1 = (Rectangle){ (float)sw/2.0f - (float)btnWidth/2.0f, (float)startY, (float)btnWidth, (float)btnHeight };
    btnEpisode2 = (Rectangle){ (float)sw/2.0f - (float)btnWidth/2.0f, (float)startY + 70, (float)btnWidth, (float)btnHeight };
    btnQuit     = (Rectangle){ (float)sw/2.0f - (float)btnWidth/2.0f, (float)startY + 140, (float)btnWidth, (float)btnHeight };
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
        if (CheckCollisionPointRec(mousePos, btnEpisode1)) {
            StartLevel(1);
        } else if (CheckCollisionPointRec(mousePos, btnEpisode2)) {
            StartLevel(2);
        } else if (CheckCollisionPointRec(mousePos, btnQuit)) {
            CloseWindow();
        }
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

    // Episode 1
    DrawRectangleRec(btnEpisode1, CheckCollisionPointRec(mousePos, btnEpisode1) ? hoverColor : normalColor);
    DrawText("EPISODE 1", btnEpisode1.x + 40, btnEpisode1.y + 15, 20, WHITE);

    // Episode 2
    DrawRectangleRec(btnEpisode2, CheckCollisionPointRec(mousePos, btnEpisode2) ? hoverColor : normalColor);
    DrawText("EPISODE 2", btnEpisode2.x + 40, btnEpisode2.y + 15, 20, WHITE);

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
        if (IsKeyPressed(KEY_R)) {
            StartLevel(currentLevel.id); // Restart current level
        }
        if (IsKeyPressed(KEY_SPACE)) {
            currentState = STATE_MENU; // Return to menu
        }
        return;
    }

    // Player Update
    UpdatePlayer(&player, &currentLevel, dt);

    // Detect equipment changes (used to kick off one-shot equip animation).
    if (player.equipmentState != lastEquipmentState) {
        if (player.equipmentState == PLAYER_EQUIP_GUN && playerGunAnimLoaded) {
            playerGunShootTimer = 0.0f;
            playerGunAnimState = PLAYER_GUN_EQUIP;
            AnimPlayer_SetClip(&playerGunAnim, &playerGunIdleClip); // takeAimGun40
            playerGunAnim.loop = false; // play once and hold last frame
        }
        lastEquipmentState = player.equipmentState;
    }
    PlayerAnimState nextState =
        Vector2Length(player.velocity) > 0.01f ? PLAYER_ANIM_WALK : PLAYER_ANIM_IDLE;
    if (nextState != playerAnimState) {
        playerAnimState = nextState;
        AnimPlayer_SetClip(&playerAnim,
                           playerAnimState == PLAYER_ANIM_WALK ? &playerWalkClip : &playerIdleClip);
    }
    AnimPlayer_Update(&playerAnim, dt);

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
    const float spriteFacingOffsetDeg = +90.0f;

    // If rotation is mirrored (turns the wrong way), flip sign:
    // float aimDeg = -atan2f(aimDirNormalized.y, aimDirNormalized.x) * RAD2DEG;
    float aimDeg = atan2f(aimDirNormalized.y, aimDirNormalized.x) * RAD2DEG;

    player.rotation = aimDeg + spriteFacingOffsetDeg;
    if (playerGunShootTimer > 0.0f) {
        playerGunShootTimer -= dt;
        if (playerGunShootTimer < 0.0f) {
            playerGunShootTimer = 0.0f;
        }
    }
    if (muzzleFlashTimer > 0.0f) {
        muzzleFlashTimer -= dt;
        if (muzzleFlashTimer < 0.0f) {
            muzzleFlashTimer = 0.0f;
        }
    }

    if (player.equipmentState == PLAYER_EQUIP_GUN && playerGunAnimLoaded) {
        // If we're in the one-shot equip anim, let it finish before entering idle/walk.
        if (playerGunAnimState == PLAYER_GUN_EQUIP) {
            AnimPlayer_Update(&playerGunAnim, dt);
            if (AnimPlayer_IsFinished(&playerGunAnim)) {
                playerGunAnimState = PLAYER_GUN_IDLE;
                AnimPlayer_SetClip(&playerGunAnim, &playerGunIdleClip);
                playerGunAnim.loop = true;
            }
        } else if (playerGunAnimState == PLAYER_GUN_RELOAD) {
            // Handle reload animation
            AnimPlayer_Update(&playerGunAnim, dt);
            
            // Transfer ammo at the right point in the animation
            if (playerGunReloadAnimLoaded && !reloadAmmoTransferred) {
                float progress = (float)playerGunAnim.frame_index / (float)playerGunReloadClip.frame_count;
                if (progress >= reloadTransferFrame) {
                    int ammoNeeded = MAG_SIZE - player.magAmmo;
                    int ammoToTransfer = ammoNeeded < player.reserveAmmo ? ammoNeeded : player.reserveAmmo;
                    player.magAmmo += ammoToTransfer;
                    player.reserveAmmo -= ammoToTransfer;
                    reloadAmmoTransferred = true;
                }
            }
            
            if (AnimPlayer_IsFinished(&playerGunAnim)) {
                playerGunAnimState = PLAYER_GUN_IDLE;
                AnimPlayer_SetClip(&playerGunAnim, &playerGunIdleClip);
                playerGunAnim.loop = true;
            }
        } else {
            PlayerGunAnimState gunNextState = PLAYER_GUN_IDLE;
            if (playerGunShootTimer > 0.0f) {
                gunNextState = PLAYER_GUN_SHOOT;
            } else if (Vector2Length(player.velocity) > 0.01f) {
                gunNextState = PLAYER_GUN_WALK;
            }
            if (gunNextState != playerGunAnimState) {
                playerGunAnimState = gunNextState;
                AnimPlayer_SetClip(&playerGunAnim,
                                   playerGunAnimState == PLAYER_GUN_SHOOT ? &playerGunShootClip :
                                   playerGunAnimState == PLAYER_GUN_WALK ? &playerGunWalkClip :
                                                                           &playerGunIdleClip);
                playerGunAnim.loop = true;
            }
            AnimPlayer_Update(&playerGunAnim, dt);
        }
    }

    // Player Shooting
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        if (player.equipmentState == PLAYER_EQUIP_GUN && HasAbility(player.identity, ABILITY_SHOOT)) {
            // Can't shoot while reloading
            if (playerGunAnimState != PLAYER_GUN_RELOAD) {
                if (player.magAmmo > 0) {
                    // Shoot bullet
                    for (int i = 0; i < MAX_BULLETS; i++) {
                        if (!bullets[i].active) {
                            bullets[i].active = true;
                            bullets[i].position = player.position;
                            bullets[i].radius = BULLET_RADIUS;
                            bullets[i].lifeTime = BULLET_LIFETIME;
                            bullets[i].isPlayerOwned = true;
                            bullets[i].velocity = Vector2Scale(aimDirNormalized, BULLET_SPEED);
                            playerGunShootTimer = playerGunShootHold;
                            muzzleFlashTimer = muzzleFlashDuration;
                            player.magAmmo--;
                            break;
                        }
                    }
                } else {
                    // Empty magazine - auto reload if we have reserve ammo
                    if (player.reserveAmmo > 0 && playerGunReloadAnimLoaded) {
                        playerGunAnimState = PLAYER_GUN_RELOAD;
                        AnimPlayer_SetClip(&playerGunAnim, &playerGunReloadClip);
                        playerGunAnim.loop = false;
                        reloadAmmoTransferred = false;
                    }
                }
            }
        }
    }

    // Reload handling
    if (IsKeyPressed(KEY_R)) {
        if (player.equipmentState == PLAYER_EQUIP_GUN && 
            playerGunAnimState != PLAYER_GUN_RELOAD &&
            player.magAmmo < MAG_SIZE && 
            player.reserveAmmo > 0 &&
            playerGunReloadAnimLoaded) {
            playerGunAnimState = PLAYER_GUN_RELOAD;
            AnimPlayer_SetClip(&playerGunAnim, &playerGunReloadClip);
            playerGunAnim.loop = false;
            reloadAmmoTransferred = false;
        }
    }

    meleeTargetIndex = -1;
    if (player.equipmentState == PLAYER_EQUIP_BARE_HANDS) {
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
            DrawText("Press SPACE to Menu", sw / 2 - 100, sh / 2 + 60, 20, RAYWHITE);
        } else {
            DrawText("EPISODE COMPLETE!", sw / 2 - 150, sh / 2 - 20, 40, GOLD);
            DrawText("Press R to Replay", sw / 2 - 100, sh / 2 + 30, 20, RAYWHITE);
            DrawText("Press SPACE to Menu", sw / 2 - 100, sh / 2 + 60, 20, RAYWHITE);
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

        if (playerAnimLoaded) {
            Texture2D frame = AnimPlayer_GetFrame(&playerAnim);
            if (player.equipmentState == PLAYER_EQUIP_GUN && playerGunAnimLoaded) {
                Texture2D gunFrame = AnimPlayer_GetFrame(&playerGunAnim);
                if (gunFrame.id != 0) {
                    frame = gunFrame;
                }
            }
            if (playerShadow.id != 0 && frame.id != 0) {
                DrawPlayerShadow(playerShadow, player.position);
            }
            if (frame.id != 0) {
                DrawPlayerFrame(frame, player.position, player.rotation);
            } else {
                DrawPlayerFallback(player.position, player.radius);
            }
            
            // Draw muzzle flash if active
            if (muzzleFlashTimer > 0.0f && muzzleFlashLoaded) {
                // Calculate muzzle flash position with offset based on aim direction
                float aimRadians = (player.rotation - 90.0f) * DEG2RAD;
                float facingX = cosf(aimRadians);
                float facingY = sinf(aimRadians);
                
                // Apply offset in the direction the gun is facing
                Vector2 flashPos = {
                    player.position.x + facingX * MUZZLE_OFFSET_X + facingY * MUZZLE_OFFSET_Y,
                    player.position.y + facingY * MUZZLE_OFFSET_X - facingX * MUZZLE_OFFSET_Y
                };
                
                float flashScale = 0.5f;
                Rectangle source = {0, 0, (float)muzzleFlashTexture.width, (float)muzzleFlashTexture.height};
                Rectangle dest = {
                    flashPos.x, 
                    flashPos.y,
                    muzzleFlashTexture.width * flashScale,
                    muzzleFlashTexture.height * flashScale
                };
                Vector2 origin = {
                    (muzzleFlashTexture.width * flashScale) / 2.0f,
                    (muzzleFlashTexture.height * flashScale) / 2.0f
                };
                
                DrawTexturePro(muzzleFlashTexture, source, dest, origin, player.rotation - 90.0f, WHITE);
            }
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
        DrawText(TextFormat("CURRENT LEVEL: %d", player.identity.permissionLevel),
                 20, 20, 20, WHITE);
        DrawText(player.equipmentState == PLAYER_EQUIP_GUN ? "EQUIP: GUN" : "EQUIP: HANDS",
                 20, 45, 16, WHITE);
        if (player.equipmentState == PLAYER_EQUIP_GUN) {
            DrawText(TextFormat("AMMO: %d / %d", player.magAmmo, player.reserveAmmo),
                     20, 70, 16, WHITE);
            if (playerGunAnimState == PLAYER_GUN_RELOAD) {
                DrawText("RELOADING...", 20, 95, 16, YELLOW);
            }
        }
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
    UnloadAnimClip(&playerIdleClip);
    UnloadAnimClip(&playerWalkClip);
    UnloadAnimClip(&playerGunIdleClip);
    UnloadAnimClip(&playerGunWalkClip);
    UnloadAnimClip(&playerGunShootClip);
    if (playerShadow.id != 0) {
        UnloadTexture(playerShadow);
        playerShadow = (Texture2D){0};
    }
    playerAnimLoaded = false;
    playerGunAnimLoaded = false;
}

