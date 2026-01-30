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

static AnimClip playerIdleClip;
static AnimClip playerWalkClip;
static AnimPlayer playerAnim;
static Texture2D playerShadow;
static bool playerAnimLoaded;
static PlayerAnimState playerAnimState;
static const float playerSpriteScale = 0.2f;

static void DrawPlayerFallback(Vector2 position, float radius) {
    float size = radius * 2.0f;
    DrawRectangleV((Vector2){position.x - radius, position.y - radius},
                   (Vector2){size, size}, RED);
}

void Game_Init(void) {
    gameOver = false;
    gameWon = false;
    maskActive = false;

    // Reset bullets
    for (int i = 0; i < MAX_BULLETS; i++) bullets[i].active = false;

    // Init Level
    InitLevel(1, &currentLevel);

    // Player
    player = InitPlayer(currentLevel.playerSpawn, currentLevel.playerStartId);

    // Camera
    camera = (Camera2D){0};
    camera.target = player.position;
    camera.offset = (Vector2){ GetScreenWidth() / 2.0f, GetScreenHeight() / 2.0f };
    camera.rotation = 0.0f;
    camera.zoom = 1.0f;

    if (!playerAnimLoaded) {
        UnloadAnimClip(&playerIdleClip);
        UnloadAnimClip(&playerWalkClip);
        if (playerShadow.id != 0) {
            UnloadTexture(playerShadow);
            playerShadow = (Texture2D){0};
        }
        playerIdleClip = LoadAnimClip("assets/character/LightArtilleryRobot/idle60", 30.0f);
        playerWalkClip = LoadAnimClip("assets/character/LightArtilleryRobot/walk60", 60.0f);
        playerShadow = LoadTexture("assets/character/LightArtilleryRobot/shadow.png");
        playerAnimLoaded = playerIdleClip.frame_count > 0 && playerWalkClip.frame_count > 0 &&
                           playerShadow.id != 0;
        playerAnim = (AnimPlayer){0};
        playerAnimState = PLAYER_ANIM_IDLE;
        AnimPlayer_SetClip(&playerAnim, &playerIdleClip);
        if (!playerAnimLoaded) {
            TraceLog(LOG_ERROR, "Player animation assets missing, using fallback");
        }
    } else {
        playerAnimState = PLAYER_ANIM_IDLE;
        AnimPlayer_SetClip(&playerAnim, &playerIdleClip);
    }
}

void Game_Update(void) {
    float dt = GetFrameTime();

    // Update Camera Offset
    camera.offset = (Vector2){ GetScreenWidth() / 2.0f, GetScreenHeight() / 2.0f };

    // Game Over / Win Logic Inputs
    if (gameOver || gameWon) {
        if (IsKeyPressed(KEY_R)) {
            Game_Init(); // Restart
        }
        return;
    }

    // Player Update
    UpdatePlayer(&player, &currentLevel, dt);
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
    player.rotation = atan2f(aimDir.y, aimDir.x) * RAD2DEG;

    // Player Shooting
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        if (HasAbility(player.identity, ABILITY_SHOOT)) {
            for (int i = 0; i < MAX_BULLETS; i++) {
                if (!bullets[i].active) {
                    bullets[i].active = true;
                    bullets[i].position = player.position;
                    bullets[i].radius = BULLET_RADIUS;
                    bullets[i].lifeTime = BULLET_LIFETIME;
                    bullets[i].isPlayerOwned = true;
                    bullets[i].velocity = Vector2Scale(Vector2Normalize(aimDir), BULLET_SPEED);
                    break;
                }
            }
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
        if (!bullets[i].active) continue;

        if (bullets[i].isPlayerOwned) {
            // Check against Enemies
            for (int e = 0; e < currentLevel.enemyCount; e++) {
                if (currentLevel.enemies[e].active &&
                    CheckCollisionCircles(bullets[i].position, bullets[i].radius,
                                          currentLevel.enemies[e].position, currentLevel.enemies[e].radius)) {
                    currentLevel.enemies[e].active = false;
                    bullets[i].active = false;
                    // Drop Mask Logic
                    maskActive = true;
                    droppedMask.identity = currentLevel.enemies[e].identity;
                    droppedMask.position = currentLevel.enemies[e].position;
                    droppedMask.radius = 15.0f;
                    droppedMask.active = true;
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
        gameWon = true;
    }
}

void Game_Draw(void) {
    BeginDrawing();
    ClearBackground((Color){20, 20, 25, 255});

    if (gameOver || gameWon) {
        int sw = GetScreenWidth();
        int sh = GetScreenHeight();

        if (gameOver) {
            DrawText("GAME OVER", sw / 2 - 100, sh / 2 - 20, 40, RED);
            DrawText("Press R to Restart Level", sw / 2 - 100, sh / 2 + 30, 20, RAYWHITE);
        } else {
            DrawText("EPISODE COMPLETE!", sw / 2 - 150, sh / 2 - 20, 40, GOLD);
            DrawText("Press R to Replay", sw / 2 - 100, sh / 2 + 30, 20, RAYWHITE);
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
            if (playerShadow.id != 0 && frame.id != 0) {
                Vector2 shadowPos = {
                    player.position.x - (playerShadow.width * playerSpriteScale) / 2.0f,
                    player.position.y - (playerShadow.height * playerSpriteScale) / 2.0f
                };
                DrawTextureEx(playerShadow, shadowPos, 0.0f, playerSpriteScale, WHITE);
            }
            if (frame.id != 0) {
                Vector2 framePos = {
                    player.position.x - (frame.width * playerSpriteScale) / 2.0f,
                    player.position.y - (frame.height * playerSpriteScale) / 2.0f
                };
                DrawTextureEx(frame, framePos, 0.0f, playerSpriteScale, WHITE);
            } else {
                DrawPlayerFallback(player.position, player.radius);
            }
        } else {
            DrawPlayerFallback(player.position, player.radius);
        }
        Vector2 mouseWorld = GetScreenToWorld2D(GetMousePosition(), camera);
        DrawLineV(player.position, mouseWorld, Fade(WHITE, 0.2f));
        EndMode2D();

        // HUD
        DrawText(TextFormat("CURRENT LEVEL: %d", player.identity.permissionLevel),
                 20, 20, 20, WHITE);
        DrawText(playerAnimState == PLAYER_ANIM_WALK ? "WALK" : "IDLE", 20, 45, 16, WHITE);

        if (gameWon) {
            int sw = GetScreenWidth();
            DrawText("WINNER!", sw / 2 - 50, 100, 40, GOLD);
        }
    }

    EndDrawing();
}

void Game_Shutdown(void) {
    UnloadAnimClip(&playerIdleClip);
    UnloadAnimClip(&playerWalkClip);
    if (playerShadow.id != 0) {
        UnloadTexture(playerShadow);
        playerShadow = (Texture2D){0};
    }
    playerAnimLoaded = false;
}
