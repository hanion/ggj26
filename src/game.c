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
    PLAYER_FEET_IDLE,
    PLAYER_FEET_WALK,
    PLAYER_FEET_RUN,
    PLAYER_FEET_STRAFE_LEFT,
    PLAYER_FEET_STRAFE_RIGHT
} PlayerFeetAnimState;

typedef enum {
    PLAYER_WEAPON_IDLE,
    PLAYER_WEAPON_MOVE,
    PLAYER_WEAPON_SHOOT,
    PLAYER_WEAPON_RELOAD,
    PLAYER_WEAPON_MELEE
} PlayerWeaponAnimState;

// Feet animations
static AnimClip feetIdleClip;
static AnimClip feetWalkClip;
static AnimClip feetRunClip;
static AnimClip feetStrafeLeftClip;
static AnimClip feetStrafeRightClip;

// Weapon animations - Handgun
static AnimClip handgunIdleClip;
static AnimClip handgunMoveClip;
static AnimClip handgunShootClip;
static AnimClip handgunReloadClip;
static AnimClip handgunMeleeClip;

// Weapon animations - Rifle
static AnimClip rifleIdleClip;
static AnimClip rifleMoveClip;
static AnimClip rifleShootClip;
static AnimClip rifleReloadClip;
static AnimClip rifleMeleeClip;

// Weapon animations - Shotgun
static AnimClip shotgunIdleClip;
static AnimClip shotgunMoveClip;
static AnimClip shotgunShootClip;
static AnimClip shotgunReloadClip;
static AnimClip shotgunMeleeClip;

// Weapon animations - Flashlight
static AnimClip flashlightIdleClip;
static AnimClip flashlightMoveClip;
static AnimClip flashlightMeleeClip;

// Weapon animations - Knife
static AnimClip knifeIdleClip;
static AnimClip knifeMoveClip;
static AnimClip knifeMeleeClip;

static AnimPlayer feetAnim;
static AnimPlayer weaponAnim;
static Texture2D playerShadow;
static bool playerAnimLoaded;
static bool weaponAnimLoaded;
static PlayerFeetAnimState feetAnimState;
static PlayerWeaponAnimState weaponAnimState;
static const float playerSpriteScale = 0.3f;
static const Vector2 playerSpritePivot = {0.5f, 0.5f};
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
        player.equipmentState = PLAYER_EQUIP_HANDGUN;
        TraceLog(LOG_INFO, "Player granted handgun after first kill");
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
    player.equipmentState = PLAYER_EQUIP_KNIFE;
    player.rotation = 0.0f;
    weaponShootTimer = 0.0f;
    lastEquipmentState = player.equipmentState;

    // Camera
    camera = (Camera2D){0};
    camera.target = player.position;
    camera.offset = (Vector2){ GetScreenWidth() / 2.0f, GetScreenHeight() / 2.0f };
    camera.rotation = 0.0f;
    camera.zoom = 1.0f;

    if (playerAnimLoaded) {
        // Unload feet animations
        UnloadAnimClip(&feetIdleClip);
        UnloadAnimClip(&feetWalkClip);
        UnloadAnimClip(&feetRunClip);
        UnloadAnimClip(&feetStrafeLeftClip);
        UnloadAnimClip(&feetStrafeRightClip);
        if (playerShadow.id != 0) {
            UnloadTexture(playerShadow);
            playerShadow = (Texture2D){0};
        }
        playerAnimLoaded = false;
    }
    if (weaponAnimLoaded) {
        // Unload handgun animations
        UnloadAnimClip(&handgunIdleClip);
        UnloadAnimClip(&handgunMoveClip);
        UnloadAnimClip(&handgunShootClip);
        UnloadAnimClip(&handgunReloadClip);
        UnloadAnimClip(&handgunMeleeClip);
        
        // Unload rifle animations
        UnloadAnimClip(&rifleIdleClip);
        UnloadAnimClip(&rifleMoveClip);
        UnloadAnimClip(&rifleShootClip);
        UnloadAnimClip(&rifleReloadClip);
        UnloadAnimClip(&rifleMeleeClip);
        
        // Unload shotgun animations
        UnloadAnimClip(&shotgunIdleClip);
        UnloadAnimClip(&shotgunMoveClip);
        UnloadAnimClip(&shotgunShootClip);
        UnloadAnimClip(&shotgunReloadClip);
        UnloadAnimClip(&shotgunMeleeClip);
        
        // Unload flashlight animations
        UnloadAnimClip(&flashlightIdleClip);
        UnloadAnimClip(&flashlightMoveClip);
        UnloadAnimClip(&flashlightMeleeClip);
        
        // Unload knife animations
        UnloadAnimClip(&knifeIdleClip);
        UnloadAnimClip(&knifeMoveClip);
        UnloadAnimClip(&knifeMeleeClip);
        
        weaponAnimLoaded = false;
    }
    
    // Load feet animations (1 frame for idle, 20 frames for others)
    feetIdleClip = LoadAnimClip("assets/better/character/feet/idle", 30.0f);
    feetWalkClip = LoadAnimClip("assets/better/character/feet/walk", 30.0f);
    feetRunClip = LoadAnimClip("assets/better/character/feet/run", 30.0f);
    feetStrafeLeftClip = LoadAnimClip("assets/better/character/feet/strafe_left", 30.0f);
    feetStrafeRightClip = LoadAnimClip("assets/better/character/feet/strafe_right", 30.0f);
    
    // Load handgun animations
    handgunIdleClip = LoadAnimClip("assets/better/character/handgun/idle", 30.0f);
    handgunMoveClip = LoadAnimClip("assets/better/character/handgun/move", 30.0f);
    handgunShootClip = LoadAnimClip("assets/better/character/handgun/shoot", 60.0f);
    handgunReloadClip = LoadAnimClip("assets/better/character/handgun/reload", 30.0f);
    handgunMeleeClip = LoadAnimClip("assets/better/character/handgun/meleeattack", 30.0f);
    
    // Load rifle animations
    rifleIdleClip = LoadAnimClip("assets/better/character/rifle/idle", 30.0f);
    rifleMoveClip = LoadAnimClip("assets/better/character/rifle/move", 30.0f);
    rifleShootClip = LoadAnimClip("assets/better/character/rifle/shoot", 60.0f);
    rifleReloadClip = LoadAnimClip("assets/better/character/rifle/reload", 30.0f);
    rifleMeleeClip = LoadAnimClip("assets/better/character/rifle/meleeattack", 30.0f);
    
    // Load shotgun animations (using rifle for now since spec doesn't list shotgun separately)
    shotgunIdleClip = LoadAnimClip("assets/better/character/rifle/idle", 30.0f);
    shotgunMoveClip = LoadAnimClip("assets/better/character/rifle/move", 30.0f);
    shotgunShootClip = LoadAnimClip("assets/better/character/rifle/shoot", 60.0f);
    shotgunReloadClip = LoadAnimClip("assets/better/character/rifle/reload", 30.0f);
    shotgunMeleeClip = LoadAnimClip("assets/better/character/rifle/meleeattack", 30.0f);
    
    // Load flashlight animations
    flashlightIdleClip = LoadAnimClip("assets/better/character/flashlight/idle", 30.0f);
    flashlightMoveClip = LoadAnimClip("assets/better/character/flashlight/move", 30.0f);
    flashlightMeleeClip = LoadAnimClip("assets/better/character/flashlight/meleeattack", 30.0f);
    
    // Load knife animations
    knifeIdleClip = LoadAnimClip("assets/better/character/knife/idle", 30.0f);
    knifeMoveClip = LoadAnimClip("assets/better/character/knife/move", 30.0f);
    knifeMeleeClip = LoadAnimClip("assets/better/character/knife/meleeattack", 30.0f);
    
    // Load shadow (TODO: update path when shadow asset is available)
    playerShadow = LoadTexture("assets/character/LightArtilleryRobot/shadow.png");
    
    playerAnimLoaded = feetIdleClip.frame_count > 0 && feetWalkClip.frame_count > 0;
    weaponAnimLoaded = handgunIdleClip.frame_count > 0 && knifeIdleClip.frame_count > 0;
    
    feetAnim = (AnimPlayer){0};
    weaponAnim = (AnimPlayer){0};
    feetAnimState = PLAYER_FEET_IDLE;
    weaponAnimState = PLAYER_WEAPON_IDLE;
    
    AnimPlayer_SetClip(&feetAnim, &feetIdleClip);
    if (weaponAnimLoaded) {
        AnimPlayer_SetClip(&weaponAnim, &knifeIdleClip);
    }
    
    playerFramePivotReady = false;
    if (playerAnimLoaded) {
        UpdatePlayerPivotFromFrame(AnimPlayer_GetFrame(&feetAnim));
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

    // Detect equipment changes
    if (player.equipmentState != lastEquipmentState) {
        weaponShootTimer = 0.0f;
        weaponAnimState = PLAYER_WEAPON_IDLE;
        // Set appropriate weapon animation clip based on new equipment
        switch (player.equipmentState) {
            case PLAYER_EQUIP_HANDGUN:
                AnimPlayer_SetClip(&weaponAnim, &handgunIdleClip);
                break;
            case PLAYER_EQUIP_RIFLE:
                AnimPlayer_SetClip(&weaponAnim, &rifleIdleClip);
                break;
            case PLAYER_EQUIP_SHOTGUN:
                AnimPlayer_SetClip(&weaponAnim, &shotgunIdleClip);
                break;
            case PLAYER_EQUIP_FLASHLIGHT:
                AnimPlayer_SetClip(&weaponAnim, &flashlightIdleClip);
                break;
            case PLAYER_EQUIP_KNIFE:
                AnimPlayer_SetClip(&weaponAnim, &knifeIdleClip);
                break;
            case PLAYER_EQUIP_BARE_HANDS:
            default:
                break;
        }
        lastEquipmentState = player.equipmentState;
    }
    // Update feet animation based on movement
    PlayerFeetAnimState nextFeetState = PLAYER_FEET_IDLE;
    if (Vector2Length(player.velocity) > 0.01f) {
        nextFeetState = PLAYER_FEET_WALK;
    }
    if (nextFeetState != feetAnimState) {
        feetAnimState = nextFeetState;
        AnimPlayer_SetClip(&feetAnim,
                           feetAnimState == PLAYER_FEET_WALK ? &feetWalkClip : &feetIdleClip);
    }
    AnimPlayer_Update(&feetAnim, dt);

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
    if (weaponShootTimer > 0.0f) {
        weaponShootTimer -= dt;
        if (weaponShootTimer < 0.0f) {
            weaponShootTimer = 0.0f;
        }
    }

    // Update weapon animations
    if (player.equipmentState != PLAYER_EQUIP_BARE_HANDS && weaponAnimLoaded) {
        PlayerWeaponAnimState weaponNextState = PLAYER_WEAPON_IDLE;
        if (weaponShootTimer > 0.0f) {
            weaponNextState = PLAYER_WEAPON_SHOOT;
        } else if (Vector2Length(player.velocity) > 0.01f) {
            weaponNextState = PLAYER_WEAPON_MOVE;
        }
        
        if (weaponNextState != weaponAnimState) {
            weaponAnimState = weaponNextState;
            // Set appropriate clip based on weapon type and state
            AnimClip *targetClip = NULL;
            switch (player.equipmentState) {
                case PLAYER_EQUIP_HANDGUN:
                    targetClip = weaponAnimState == PLAYER_WEAPON_SHOOT ? &handgunShootClip :
                                 weaponAnimState == PLAYER_WEAPON_MOVE ? &handgunMoveClip :
                                 &handgunIdleClip;
                    break;
                case PLAYER_EQUIP_RIFLE:
                    targetClip = weaponAnimState == PLAYER_WEAPON_SHOOT ? &rifleShootClip :
                                 weaponAnimState == PLAYER_WEAPON_MOVE ? &rifleMoveClip :
                                 &rifleIdleClip;
                    break;
                case PLAYER_EQUIP_SHOTGUN:
                    targetClip = weaponAnimState == PLAYER_WEAPON_SHOOT ? &shotgunShootClip :
                                 weaponAnimState == PLAYER_WEAPON_MOVE ? &shotgunMoveClip :
                                 &shotgunIdleClip;
                    break;
                case PLAYER_EQUIP_FLASHLIGHT:
                    targetClip = weaponAnimState == PLAYER_WEAPON_SHOOT ? &flashlightMeleeClip :
                                 weaponAnimState == PLAYER_WEAPON_MOVE ? &flashlightMoveClip :
                                 &flashlightIdleClip;
                    break;
                case PLAYER_EQUIP_KNIFE:
                    targetClip = weaponAnimState == PLAYER_WEAPON_SHOOT ? &knifeMeleeClip :
                                 weaponAnimState == PLAYER_WEAPON_MOVE ? &knifeMoveClip :
                                 &knifeIdleClip;
                    break;
                default:
                    break;
            }
            if (targetClip != NULL) {
                AnimPlayer_SetClip(&weaponAnim, targetClip);
                weaponAnim.loop = true;
            }
        }
        AnimPlayer_Update(&weaponAnim, dt);
    }

    // Player Shooting
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        bool canShoot = (player.equipmentState == PLAYER_EQUIP_HANDGUN ||
                        player.equipmentState == PLAYER_EQUIP_RIFLE ||
                        player.equipmentState == PLAYER_EQUIP_SHOTGUN) &&
                       HasAbility(player.identity, ABILITY_SHOOT);
        if (canShoot) {
            for (int i = 0; i < MAX_BULLETS; i++) {
                if (!bullets[i].active) {
                    bullets[i].active = true;
                    bullets[i].position = player.position;
                    bullets[i].radius = BULLET_RADIUS;
                    bullets[i].lifeTime = BULLET_LIFETIME;
                    bullets[i].isPlayerOwned = true;
                    bullets[i].velocity = Vector2Scale(aimDirNormalized, BULLET_SPEED);
                    weaponShootTimer = weaponShootHold;
                    break;
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
            Texture2D feetFrame = AnimPlayer_GetFrame(&feetAnim);
            Texture2D weaponFrame = (Texture2D){0};
            
            if (player.equipmentState != PLAYER_EQUIP_BARE_HANDS && weaponAnimLoaded) {
                weaponFrame = AnimPlayer_GetFrame(&weaponAnim);
            }
            
            // Draw shadow
            if (playerShadow.id != 0 && feetFrame.id != 0) {
                DrawPlayerShadow(playerShadow, player.position);
            }
            
            // Draw feet
            if (feetFrame.id != 0) {
                DrawPlayerFrame(feetFrame, player.position, player.rotation);
            }
            
            // Draw weapon on top of feet
            if (weaponFrame.id != 0) {
                DrawPlayerFrame(weaponFrame, player.position, player.rotation);
            }
            
            // Fallback if no frames loaded
            if (feetFrame.id == 0 && weaponFrame.id == 0) {
                DrawPlayerFallback(player.position, player.radius);
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
        const char *equipText = "EQUIP: HANDS";
        switch (player.equipmentState) {
            case PLAYER_EQUIP_KNIFE: equipText = "EQUIP: KNIFE"; break;
            case PLAYER_EQUIP_FLASHLIGHT: equipText = "EQUIP: FLASHLIGHT"; break;
            case PLAYER_EQUIP_HANDGUN: equipText = "EQUIP: HANDGUN"; break;
            case PLAYER_EQUIP_RIFLE: equipText = "EQUIP: RIFLE"; break;
            case PLAYER_EQUIP_SHOTGUN: equipText = "EQUIP: SHOTGUN"; break;
            default: break;
        }
        DrawText(equipText, 20, 45, 16, WHITE);
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
    // Unload feet animations
    UnloadAnimClip(&feetIdleClip);
    UnloadAnimClip(&feetWalkClip);
    UnloadAnimClip(&feetRunClip);
    UnloadAnimClip(&feetStrafeLeftClip);
    UnloadAnimClip(&feetStrafeRightClip);
    
    // Unload weapon animations
    UnloadAnimClip(&handgunIdleClip);
    UnloadAnimClip(&handgunMoveClip);
    UnloadAnimClip(&handgunShootClip);
    UnloadAnimClip(&handgunReloadClip);
    UnloadAnimClip(&handgunMeleeClip);
    
    UnloadAnimClip(&rifleIdleClip);
    UnloadAnimClip(&rifleMoveClip);
    UnloadAnimClip(&rifleShootClip);
    UnloadAnimClip(&rifleReloadClip);
    UnloadAnimClip(&rifleMeleeClip);
    
    UnloadAnimClip(&shotgunIdleClip);
    UnloadAnimClip(&shotgunMoveClip);
    UnloadAnimClip(&shotgunShootClip);
    UnloadAnimClip(&shotgunReloadClip);
    UnloadAnimClip(&shotgunMeleeClip);
    
    UnloadAnimClip(&flashlightIdleClip);
    UnloadAnimClip(&flashlightMoveClip);
    UnloadAnimClip(&flashlightMeleeClip);
    
    UnloadAnimClip(&knifeIdleClip);
    UnloadAnimClip(&knifeMoveClip);
    UnloadAnimClip(&knifeMeleeClip);
    
    if (playerShadow.id != 0) {
        UnloadTexture(playerShadow);
        playerShadow = (Texture2D){0};
    }
    playerAnimLoaded = false;
    weaponAnimLoaded = false;
}

