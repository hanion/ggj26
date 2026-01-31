// Standard
#include <math.h>
#include <stdio.h>

// External
#include "../raylib/src/raylib.h"
#include "../raylib/src/raymath.h"
#include "../raylib/src/rlgl.h"

// Core & Context
#include "types.h"
#include "game.h"
#include "game_context.h"
#include "entity.h"
#include "levels.h"
#include "anim.h"
#include "gameplay_helpers.h"

// Game Modules
#include "episodes/episodes.h"
#include "enemies/enemy.h"
#include "player/player.h"
#include "player/player_actions.h"
#include "player/player_render.h"
#include "ui/hud.h"
#include "types.h"

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

static GameState currentState;
static Level currentLevel;
static bool gameOver; 
static bool gameWon; 

static Entity player;
static Camera2D camera;

static Bullet bullets[MAX_BULLETS];
#define MAX_MASKS 20
static Entity droppedMasks[MAX_MASKS];
static float levelStartTimer = 0.0f;
static const float LEVEL_START_DELAY = 1.0f;

static GameContext gameCtx;


static Texture2D texZone1;
static Texture2D texZone2;
static Texture2D texZone3;
static Texture2D texZone4;
static Texture2D texZone5;
static Texture2D texZone6;
static Texture2D texZone7;

// --- RENDER & GAMEPLAY STATES ---
static PlayerRender playerRender;
static float weaponShootTimer = 0.0f;
static const float weaponShootHold = 0.2f;
static PlayerEquipState lastEquipmentState = PLAYER_EQUIP_KNIFE;


static int meleeTargetIndex = -1;
static float meleeRange = 80.0f;
static float meleePromptOffset = 40.0f;
static float meleePromptHorizontalOffset = 40.0f;
static float droppedMaskRadius = 15.0f;

// Debug
static bool playerDebugDraw = false;
static float playerDebugCrossSize = 10.0f;

// Menu Buttons
static Rectangle btnEpisode1;
static Rectangle btnEpisode2;
static Rectangle btnQuit;
static Rectangle btnPlay;
static Rectangle btnContinue;

// Helper function to simulate DrawTextureTiled
void DrawTextureTiled(Texture2D texture, Rectangle source, Rectangle dest, Vector2 origin, float rotation, float scale, Color tint) {
    if ((texture.id <= 0) || (scale <= 0.0f)) return;  // Wav, what about 0 scale? w/e
    
    // Grid size
    int tileWidth = (int)((float)source.width * scale);
    int tileHeight = (int)((float)source.height * scale);
    
    if ((dest.width < tileWidth) && (dest.height < tileHeight)) {
        // Can fit one tile partially
        DrawTexturePro(texture, (Rectangle){source.x, source.y, ((float)dest.width/tileWidth)*source.width, ((float)dest.height/tileHeight)*source.height},
            (Rectangle){dest.x + origin.x, dest.y + origin.y, dest.width, dest.height}, origin, rotation, tint);
    } else {
        int dx = 0;
        int dy = 0;
        
        while (dy < (int)dest.height) {
            dx = 0;
            while (dx < (int)dest.width) {
                 // Calculate Draw Area
                 float drawW = (float)tileWidth;
                 float drawH = (float)tileHeight;
                 
                 if (dx + tileWidth > dest.width) drawW = (float)(dest.width - dx);
                 if (dy + tileHeight > dest.height) drawH = (float)(dest.height - dy);
                 
                 // Draw part of texture
                 DrawTexturePro(texture, 
                     (Rectangle){source.x, source.y, (drawW/tileWidth)*source.width, (drawH/tileHeight)*source.height},
                     (Rectangle){dest.x + dx + origin.x, dest.y + dy + origin.y, drawW, drawH},
                     (Vector2){0, 0}, rotation, tint);

                 dx += tileWidth;
            }
            dy += tileHeight;
        }
    }
}
// Removed old Game_Init logic


// --- FOG OF WAR STATE ---
static bool visitedZones[7];

void StartLevel(int id) {
    gameOver = false;
    gameWon = false;
    gameCtx.hasWonLastEpisode = false;
    
    // Reset masks
    for(int i=0; i<MAX_MASKS; i++) droppedMasks[i].active = false;
    
    currentState = STATE_PLAYING;
    levelStartTimer = LEVEL_START_DELAY;

    // Reset bullets
    for (int i = 0; i < MAX_BULLETS; i++) bullets[i].active = false;

    // Init Level
    InitLevel(id, &currentLevel);

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
    bool shouldCarryPlayer = gameCtx.player.valid && (id == gameCtx.nextEpisodeId);
    if (shouldCarryPlayer) {
        player.identity = gameCtx.player.identity;
        player.magAmmo = gameCtx.player.magAmmo;
        player.reserveAmmo = gameCtx.player.reserveAmmo;
        player.equipmentState = GameContext_AllowsEquip(&gameCtx, gameCtx.player.equipped)
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
        gameCtx.player.hasFlashlight = true; 
        gameCtx.player.hasHandgun = false;
        gameCtx.player.hasRifle = false;
        gameCtx.player.hasShotgun = false;
    }
    lastEquipmentState = player.equipmentState;
    
    // Reset Visited Zones (Fog of War)
    for (int i = 0; i < 7; i++) visitedZones[i] = false;
    visitedZones[0] = true; 

    // Camera
    camera = (Camera2D){0};
    camera.target = player.position;
    camera.offset = (Vector2){ GetScreenWidth() / 2.0f, GetScreenHeight() / 2.0f };
    camera.rotation = 0.0f;
    camera.zoom = 1.0f;

    // Load Textures
    if (texZone1.id == 0)texZone1 = LoadTexture("assets/environment/background_1.png");
    if (texZone2.id == 0)texZone2 = LoadTexture("assets/environment/background_2.png");
    if (texZone3.id == 0)texZone3 = LoadTexture("assets/environment/background_3.png");
    if (texZone4.id == 0)texZone4 = LoadTexture("assets/environment/background_4.png");
    if (texZone5.id == 0)texZone5 = LoadTexture("assets/environment/background_5.png");
    if (texZone6.id == 0)texZone6 = LoadTexture("assets/environment/background_6.png");
    if (texZone7.id == 0)texZone7 = LoadTexture("assets/environment/background_7.png");

    // Init Player Render
    PlayerRender_Init(&playerRender);
    PlayerRender_LoadEpisodeAssets(&playerRender);
    PlayerRender_OnEquip(&playerRender, player.equipmentState);
}

void Game_Init(void) {
    currentState = STATE_MENU;
    
    // Start a fresh story by default.
    GameContext_Init(&gameCtx);

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

static bool UpdateMenu(void) {
    Vector2 mousePos = GetScaledMousePosition();

    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        if (CheckCollisionPointRec(mousePos, btnEpisode1)) {
            StartLevel(1);
        } else if (CheckCollisionPointRec(mousePos, btnEpisode2)) {
            StartLevel(2);
        } else if (CheckCollisionPointRec(mousePos, btnQuit)) {
            return false;
        }
    }
    return true;
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

    // Debug Toggle
    if (IsKeyPressed(KEY_F1)) {
        playerDebugDraw = !playerDebugDraw;
    }

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
            GameContext_SaveFromPlayer(&gameCtx, &player);
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

    // Level Start Countdown
    if (levelStartTimer > 0) {
        levelStartTimer -= dt;
        return; // Don't update player or enemies yet
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
    const float spriteFacingOffsetDeg = 0.0f;

    float aimDeg = atan2f(aimDirNormalized.y, aimDirNormalized.x) * RAD2DEG;

    player.rotation = aimDeg + spriteFacingOffsetDeg;
    if (weaponShootTimer > 0.0f) {
        weaponShootTimer -= dt;
        if (weaponShootTimer < 0.0f) {
            weaponShootTimer = 0.0f;
        }
    }

    // Update all player visual animation state
    PlayerRender_Update(&playerRender, &player, dt, weaponShootTimer);
    
    // Handle reload timer
    if (player.isReloading) {
        player.reloadTimer -= dt;
        if (player.reloadTimer <= 0.0f) {
            // Reload complete
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
    bool shootPressed = IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
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
                // Auto-reload
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
        meleeTargetIndex = PlayerActions_GetClosestEnemyInRange(&currentLevel, player.position, meleeRange);
        if (meleeTargetIndex != -1 && IsKeyPressed(KEY_E)) {
            PlayerActions_HandleEnemyKilled(&currentLevel,
                                            meleeTargetIndex,
                                            &player,
                                            droppedMasks,
                                            MAX_MASKS,
                                            droppedMaskRadius);
            meleeTargetIndex = -1;
        }
    }

    // Enemy Update
    for (int i = 0; i < currentLevel.enemyCount; i++) {
        UpdateEnemy(&currentLevel.enemies[i], player.position, &currentLevel, bullets, MAX_BULLETS, dt);
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
                    PlayerActions_HandleEnemyKilled(&currentLevel,
                                                    e,
                                                    &player,
                                                    droppedMasks,
                                                    MAX_MASKS,
                                                    droppedMaskRadius);
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
    for (int i = 0; i < MAX_MASKS; i++) {
        if (droppedMasks[i].active) {
            if (CheckCollisionCircles(player.position, player.radius, droppedMasks[i].position, droppedMasks[i].radius)) {
                if (IsKeyPressed(KEY_SPACE)) {
                    player.identity = droppedMasks[i].identity;
                    droppedMasks[i].active = false;
                }
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
    if (player.position.y > 2200.0f) {
        gameWon = true;
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

        // Zone 1
        DrawTexturePro(texZone1, (Rectangle){0,0,texZone1.width,texZone1.height}, (Rectangle){0,0,913,642}, (Vector2){0,0}, 0.f, WHITE);
        // Zone 2
        DrawTexturePro(texZone2, (Rectangle){0,0,texZone2.width,texZone2.height}, (Rectangle){913,0,911,661}, (Vector2){0,0}, 0.f, WHITE);
        // Zone 3
        DrawTexturePro(texZone3, (Rectangle){0,0,texZone3.width,texZone3.height}, (Rectangle){1824,0,957,654}, (Vector2){0,0}, 0.f, WHITE);
        // Zone 4
        DrawTexturePro(texZone4, (Rectangle){0,0,texZone4.width,texZone4.height}, (Rectangle){1824,654,869,645}, (Vector2){0,0}, 0.f, WHITE);
        // Zone 5
        DrawTexturePro(texZone5, (Rectangle){0,0,texZone5.width,texZone5.height}, (Rectangle){957,654,867,649}, (Vector2){0,0}, 0.f, WHITE);
        // Zone 6
        DrawTexturePro(texZone6, (Rectangle){0,0,texZone6.width,texZone6.height}, (Rectangle){162,654,795,853}, (Vector2){0,0}, 0.f, WHITE);
        // Zone 7
        DrawTexturePro(texZone7, (Rectangle){0,0,texZone7.width,texZone7.height}, (Rectangle){162,1507,795,805}, (Vector2){0,0}, 0.f, WHITE);

        // Draw Level Elements
        if (playerDebugDraw) {
            for (int i = 0; i < currentLevel.wallCount; i++) DrawRectangleRec(currentLevel.walls[i], Fade(RED, 0.5f));
        }
        for (int i = 0; i < currentLevel.doorCount; i++) {
            Color doorColor = SKYBLUE;
            if (currentLevel.doorPerms[i] == PERM_STAFF) doorColor = GREEN;
            else if (currentLevel.doorPerms[i] == PERM_GUARD) doorColor = RED;
            else if (currentLevel.doorPerms[i] == PERM_ADMIN) doorColor = PURPLE;

            if (!currentLevel.doorsOpen[i]) {
                DrawRectangleRec(currentLevel.doors[i], Fade(doorColor, 0.6f));
                DrawRectangleLinesEx(currentLevel.doors[i], 2.0f, WHITE);
                
                // Draw a small lock icon visual (circle)
                Vector2 center = { currentLevel.doors[i].x + currentLevel.doors[i].width/2, 
                                   currentLevel.doors[i].y + currentLevel.doors[i].height/2 };
                DrawCircleV(center, 4.0f, WHITE);
            } else {
                DrawRectangleLinesEx(currentLevel.doors[i], 3.0f, Fade(doorColor, 0.5f));
            }
        }


        if (currentLevel.id == 1) {
             DrawText("ZONE 1: STAFF ONLY", 400, 300, 30, Fade(WHITE, 0.1f));
             // Animated "EXIT" Text
             float time = (float)GetTime();
             float rotation = sinf(time * 2.0f) * 10.0f; // Rock back and forth +/- 10 degrees
             float scale = 1.0f + sinf(time * 5.0f) * 0.1f; // Pulse scale
             
             // Burning/Flashing Color
             Color c1 = ORANGE;
             Color c2 = RED;
             float t = (sinf(time * 8.0f) + 1.0f) / 2.0f;
             Color burnColor = (Color){
                (unsigned char)(c1.r + t*(c2.r - c1.r)),
                (unsigned char)(c1.g + t*(c2.g - c1.g)),
                (unsigned char)(c1.b + t*(c2.b - c1.b)),
                255
             };

             Vector2 textSize = MeasureTextEx(GetFontDefault(), "EXIT", 40, 4);
             Vector2 origin = { textSize.x / 2, textSize.y / 2 };
             DrawTextPro(GetFontDefault(), "EXIT", (Vector2){480 + origin.x, 2340 + origin.y}, origin, rotation, 40 * scale, 4, burnColor);
        }

        // Enemies
        for (int i = 0; i < currentLevel.enemyCount; i++) {
            if (currentLevel.enemies[i].active) {
                // Draw Vision Cone
                float halfAngle = currentLevel.enemies[i].sightAngle / 2.0f;
                Vector2 origin = currentLevel.enemies[i].position;
                float startAngle = currentLevel.enemies[i].rotation - halfAngle;
                float endAngle = currentLevel.enemies[i].rotation + halfAngle;
                int segments = 30; 
                float step = (endAngle - startAngle) / segments;
                
                rlSetTexture(0);
                rlDisableBackfaceCulling(); // Ensure we see it regardless of winding
                rlBegin(RL_TRIANGLES);
                rlColor4ub(200, 200, 200, 60); // Light Gray, Semi-transparent

                for (int s = 0; s < segments; s++) {
                    float a1 = (startAngle + s * step) * DEG2RAD;
                    float a2 = (startAngle + (s + 1) * step) * DEG2RAD;

                    Vector2 d1 = { cosf(a1) * currentLevel.enemies[i].sightRange, sinf(a1) * currentLevel.enemies[i].sightRange };
                    Vector2 d2 = { cosf(a2) * currentLevel.enemies[i].sightRange, sinf(a2) * currentLevel.enemies[i].sightRange };

                    Vector2 p1 = Vector2Add(origin, d1);
                    Vector2 p2 = Vector2Add(origin, d2);
                    
                    // Raycast against walls
                    p1 = Gameplay_GetRayHit(origin, p1, &currentLevel);
                    p2 = Gameplay_GetRayHit(origin, p2, &currentLevel);

                    // Draw Triangle (Origin -> P1 -> P2)
                    rlVertex2f(origin.x, origin.y);
                    rlVertex2f(p1.x, p1.y);
                    rlVertex2f(p2.x, p2.y);
                    
                    // Draw Backface just in case
                    rlVertex2f(origin.x, origin.y);
                    rlVertex2f(p2.x, p2.y);
                    rlVertex2f(p1.x, p1.y);
                }
                rlEnd();
                rlEnableBackfaceCulling(); // Reset default (though usually off in 2D)

                DrawCircleV(currentLevel.enemies[i].position, currentLevel.enemies[i].radius, currentLevel.enemies[i].identity.color);
                DrawCircleLines((int)currentLevel.enemies[i].position.x, (int)currentLevel.enemies[i].position.y, currentLevel.enemies[i].radius + 2, WHITE);
            }
        }

        // Mask
        for (int i = 0; i < MAX_MASKS; i++) {
            if (droppedMasks[i].active) {
                DrawCircleV(droppedMasks[i].position, droppedMasks[i].radius, droppedMasks[i].identity.color);
                DrawText("PRESS SPACE", (int)droppedMasks[i].position.x - 30, (int)droppedMasks[i].position.y - 30, 10, WHITE);
            }
        }

        // Bullets
        for (int i = 0; i < MAX_BULLETS; i++) {
            if (bullets[i].active) DrawCircleV(bullets[i].position, bullets[i].radius, bullets[i].isPlayerOwned ? YELLOW : ORANGE);
        }

        // Player
        if (playerRender.loaded) {
            PlayerRender_Draw(&playerRender, &player);
            PlayerRender_DrawMuzzleFlash(&playerRender, &player, weaponShootTimer);
        } else {
            PlayerRender_DrawFallback(player.position, player.radius); // Fallback if not loaded
        }

        // Melee Prompt
        if (meleeTargetIndex >= 0 && meleeTargetIndex < currentLevel.enemyCount && currentLevel.enemies[meleeTargetIndex].active) {
             DrawText("PRESS E TO CHOKE", (int)player.position.x, (int)player.position.y - 60, 12, WHITE);
        }
        
        // Debug
        if (playerDebugDraw) {
             DrawCircleLines((int)player.position.x, (int)player.position.y, player.radius, GOLD);
        }
        
        EndMode2D();
    // HUD
    Hud_DrawPlayer(&player);

    if (levelStartTimer > 0) {
        DrawText("READY...", GetScreenWidth()/2 - 50, GetScreenHeight()/2, 30, RED);
    }
    }
    EndDrawing();
}


bool Game_Update(void) {
    if (currentState == STATE_MENU) {
        return UpdateMenu();
    } else {
        UpdateGame(GetFrameTime());
        return true;
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
    UnloadTexture(texZone1);
    UnloadTexture(texZone2);
    UnloadTexture(texZone3);
    UnloadTexture(texZone4);
    UnloadTexture(texZone5);
    UnloadTexture(texZone6);
    UnloadTexture(texZone7);
}

