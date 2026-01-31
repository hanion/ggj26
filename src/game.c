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
static Bullet bullets[MAX_BULLETS];
static Entity droppedMask;
static bool maskActive;


static Texture2D texZone1;
static Texture2D texZone2;
static Texture2D texZone3;
static Texture2D texZone4;
static Texture2D texZone5;
static Texture2D texZone6;
static Texture2D texZone7;

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
void Game_Init(void) {
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
    playerGunShootTimer = 0.0f;
    lastEquipmentState = player.equipmentState;

    // Camera
    camera = (Camera2D){0};
    camera.target = player.position;
    camera.offset = (Vector2){ GetScreenWidth() / 2.0f, GetScreenHeight() / 2.0f };
    camera.rotation = 0.0f;
    camera.zoom = 1.0f;
    camera.rotation = 0.0f;
    camera.zoom = 1.0f;

    // Load Textures (make sure to move these to assets/environment/...)
    // Load Textures
    texZone1 = LoadTexture("assets/environment/background_1.png");
    texZone2 = LoadTexture("assets/environment/background_2.png");
    texZone3 = LoadTexture("assets/environment/background_3.png");
    texZone4 = LoadTexture("assets/environment/background_4.png");
    texZone5 = LoadTexture("assets/environment/background_5.png");
    texZone6 = LoadTexture("assets/environment/background_6.png");
    texZone7 = LoadTexture("assets/environment/background_7.png");
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

    if (player.equipmentState == PLAYER_EQUIP_GUN && playerGunAnimLoaded) {
        // If we're in the one-shot equip anim, let it finish before entering idle/walk.
        if (playerGunAnimState == PLAYER_GUN_EQUIP) {
            AnimPlayer_Update(&playerGunAnim, dt);
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
            for (int i = 0; i < MAX_BULLETS; i++) {
                if (!bullets[i].active) {
                    bullets[i].active = true;
                    bullets[i].position = player.position;
                    bullets[i].radius = BULLET_RADIUS;
                    bullets[i].lifeTime = BULLET_LIFETIME;
                    bullets[i].isPlayerOwned = true;
                    bullets[i].velocity = Vector2Scale(aimDirNormalized, BULLET_SPEED);
                    playerGunShootTimer = playerGunShootHold;
                    break;
                }
            }
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
    for (int i = 0; i < currentLevel.enemyCount; i++) {
        UpdateEnemy(&currentLevel.enemies[i], player.position, bullets, MAX_BULLETS, dt, &currentLevel);
    }
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
    // Snake Layout ends at Z7 (bottom-left area, y=1507..2312)
    // Reach exit at bottom (y > 2200)
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
            DrawText("Press SPACE to Menu", sw / 2 - 100, sh / 2 + 60, 20, RAYWHITE);
        } else {
            DrawText("EPISODE COMPLETE!", sw / 2 - 150, sh / 2 - 20, 40, GOLD);
            DrawText("Press R to Replay", sw / 2 - 100, sh / 2 + 30, 20, RAYWHITE);
            DrawText("Press SPACE to Menu", sw / 2 - 100, sh / 2 + 60, 20, RAYWHITE);
        }
    } else {
        BeginMode2D(camera);

        // Draw Tiled Backgrounds for each Zone
        // DrawTextureTiled(texture, source, dest, origin, rotation, scale, tint)
        
        // Zone 1: Staff (0 to 1000)
        // Draw Scaled Backgrounds for each Zone (Exact Sizes)
        // DrawTexturePro(texture, source, dest, origin, rotation, tint)
        
        // Zone 1: (0, 0) -> 913x642
        DrawTexturePro(texZone1, 
            (Rectangle){0, 0, (float)texZone1.width, (float)texZone1.height}, 
            (Rectangle){0, 0, 913, 642}, 
            (Vector2){0, 0}, 0.0f, WHITE);

        // Zone 2: (913, 0) -> 911x661
        DrawTexturePro(texZone2, 
            (Rectangle){0, 0, (float)texZone2.width, (float)texZone2.height}, 
            (Rectangle){913, 0, 911, 661}, 
            (Vector2){0, 0}, 0.0f, WHITE);

        // Zone 3: (1824, 0) -> 957x654 (913+911=1824)
        DrawTexturePro(texZone3, 
            (Rectangle){0, 0, (float)texZone3.width, (float)texZone3.height}, 
            (Rectangle){1824, 0, 957, 654}, 
            (Vector2){0, 0}, 0.0f, WHITE);

        // Zone 4: (1824, 654) -> 869x645 (Below Z3 Left)
        DrawTexturePro(texZone4, 
            (Rectangle){0, 0, (float)texZone4.width, (float)texZone4.height}, 
            (Rectangle){1824, 654, 869, 645}, 
            (Vector2){0, 0}, 0.0f, WHITE);

        // Zone 5: (957, 654) -> 867x649 (Left of Z4: 1824-867=957)
        DrawTexturePro(texZone5, 
            (Rectangle){0, 0, (float)texZone5.width, (float)texZone5.height}, 
            (Rectangle){957, 654, 867, 649}, 
            (Vector2){0, 0}, 0.0f, WHITE);

        // Zone 6: (162, 654) -> 795x853 (Left of Z5: 957-795=162)
        DrawTexturePro(texZone6, 
            (Rectangle){0, 0, (float)texZone6.width, (float)texZone6.height}, 
            (Rectangle){162, 654, 795, 853}, 
            (Vector2){0, 0}, 0.0f, WHITE);

        // Zone 7: (162, 1507) -> 795x805 (Below Z6: 654+853=1507)
        DrawTexturePro(texZone7, 
            (Rectangle){0, 0, (float)texZone7.width, (float)texZone7.height}, 
            (Rectangle){162, 1507, 795, 805}, 
            (Vector2){0, 0}, 0.0f, WHITE);
        // Draw Level Elements
        for (int i = 0; i < currentLevel.wallCount; i++)
            DrawRectangleRec(currentLevel.walls[i], GRAY);
        for (int i = 0; i < currentLevel.doorCount; i++) {
            Color dColor = currentLevel.doorsOpen[i] ? GREEN : DARKGRAY;
            if (!currentLevel.doorsOpen[i]) {
                DrawRectangleRec(currentLevel.doors[i], dColor);
                const char* permName = "UNKNOWN";
                switch(currentLevel.doorPerms[i]) {
                    case PERM_CIVILIAN: permName = "REQ: CIV"; break;
                    case PERM_STAFF:    permName = "REQ: STAFF"; break;
                    case PERM_GUARD:    permName = "REQ: GUARD"; break;
                    case PERM_ADMIN:    permName = "REQ: ADMIN"; break;
                }
                DrawText(permName,
                         (int)currentLevel.doors[i].x - 40,
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
    UnloadTexture(texZone1);
    UnloadTexture(texZone2);
    UnloadTexture(texZone3);
    UnloadTexture(texZone4);
    UnloadTexture(texZone5);
    UnloadTexture(texZone6);
    UnloadTexture(texZone7);
}

