// Standard
#include <math.h>

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
#include "gameplay_helpers.h"

// Game Modules
#include "enemies/enemy.h"
#include "player/player.h"
#include "player/player_actions.h"
#include "player/player_render.h"
#include "ui/hud.h"
#include "types.h"

#include "editor.c"

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
LevelEditor editor;
static bool gameOver; 
static bool gameWon; 

static Entity player;
static Camera2D camera;

static Bullet bullets[MAX_BULLETS];
#define MAX_MASKS 20
static Entity droppedMasks[MAX_MASKS];
static float levelStartTimer = 0.0f;
static const float LEVEL_START_DELAY = 1.0f;
#define MAX_CARDS 10
static Entity droppedCards[MAX_CARDS];

#define MAX_DROPPED_GUNS 20
static DroppedGun droppedGuns[MAX_DROPPED_GUNS];

// Sounds
static Sound fxShoot = {0};
static Sound fxReload = {0};

// --- BLOOD PARTICLES ---
#define MAX_PARTICLES 200
typedef struct {
    Vector2 position;
    Vector2 velocity;
    float life; // 0.0 to 1.0
    Color color;
    bool active;
    float size;
} Particle;

static Particle particles[MAX_PARTICLES];

static GameContext gameCtx;

// Forward Declarations
static void DrawGame(void);
static void UpdateGame(float dt);
static PlayerEquipState MapGunToEquip(GunType type);



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

void EndLevel(int id);

void StartLevel(int id) {
	if (id) {
		EndLevel(id);
	}
	editor = LevelEditor_new(&currentLevel);
    gameOver = false;
    gameWon = false;
    gameCtx.hasWonLastEpisode = false;
    
    // Reset masks
    for(int i=0; i<MAX_MASKS; i++) droppedMasks[i].active = false;
    // Reset cards
    for(int i=0; i<MAX_CARDS; i++) droppedCards[i].active = false;
    // Reset guns
    for(int i=0; i<MAX_DROPPED_GUNS; i++) droppedGuns[i].active = false;
    
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
    // carry identity/inventory across.
    bool shouldCarryPlayer = gameCtx.player.valid && (id == gameCtx.nextEpisodeId);
    if (shouldCarryPlayer) {
        player.identity = gameCtx.player.identity;
        player.inventory = gameCtx.player.inventory;
    } else {
        // Fresh episode start (new game / replay): defaults handled by InitPlayer
        // But we want to sync the Context if it wasn't valid
        gameCtx.player.valid = true;
        gameCtx.player.identity = player.identity;
        gameCtx.player.inventory = player.inventory;
    }
    // lastEquipmentState legacy check, maybe track GunType instead?
    // lastEquipmentState = ...? Let's just reset timer
    weaponShootTimer = 0.0f;
    GunType startGun = player.inventory.gunSlots[player.inventory.currentGunIndex].type;
    lastEquipmentState = MapGunToEquip(startGun);
    
    // Reset Visited Zones (Fog of War)
    for (int i = 0; i < 7; i++) visitedZones[i] = false;
    visitedZones[0] = true; 

    // Camera
    camera = (Camera2D){0};
    camera.target = player.position;
    camera.offset = (Vector2){ GetScreenWidth() / 2.0f, GetScreenHeight() / 2.0f };
    camera.rotation = 0.0f;
    camera.zoom = 1.0f;

    // Init Player Render
    PlayerRender_Init(&playerRender);
    PlayerRender_LoadEpisodeAssets(&playerRender);
    PlayerRender_OnEquip(&playerRender, lastEquipmentState);
}

void EndLevel(int id) {
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

// Developer Mode
static bool developerMode = false;

// Helper to map GunType to legacy PlayerEquipState for rendering
static PlayerEquipState MapGunToEquip(GunType type) {
    if (type == GUN_HANDGUN) return PLAYER_EQUIP_HANDGUN;
    if (type == GUN_RIFLE) return PLAYER_EQUIP_RIFLE;
    if (type == GUN_SHOTGUN) return PLAYER_EQUIP_SHOTGUN;
    if (type == GUN_KNIFE) return PLAYER_EQUIP_KNIFE;
    return PLAYER_EQUIP_BARE_HANDS; 
}

static void SpawnBlood(Vector2 pos, int count) {
    for (int i = 0; i < count; i++) {
        // Find empty slot
        int slot = -1;
        for (int p=0; p<MAX_PARTICLES; p++) {
            if (!particles[p].active) {
                slot = p;
                break;
            }
        }
        
        if (slot != -1) {
            particles[slot].active = true;
            particles[slot].position = pos;
            
            // Random direction blood spray
            float angle = (float)GetRandomValue(0, 360) * DEG2RAD;
            float speed = (float)GetRandomValue(100, 300);
            
            particles[slot].velocity = (Vector2){ cosf(angle)*speed, sinf(angle)*speed };
            particles[slot].life = 1.0f; // 1 second
            particles[slot].size = (float)GetRandomValue(2, 5);
            particles[slot].color = (Color){ 200, 0, 0, 255 }; // Deep Red
        }
    }
}

static void UpdateGame(float dt) {
	camera.offset = (Vector2){ GetScreenWidth() / 2.0f, GetScreenHeight() / 2.0f };
	float scale_x = GetScreenWidth()  / 1920.0f;
	float scale_y = GetScreenHeight() / 1080.0f;
	camera.zoom = 1.5f*fminf(scale_x, scale_y);


    // Debug Toggle F1
    if (IsKeyPressed(KEY_F1)) {
        playerDebugDraw = !playerDebugDraw;
    }
    
    // Developer Mode Toggle F
    if (IsKeyPressed(KEY_F)) {
        developerMode = !developerMode;
    }
    
    // Update Particles
    for(int i=0; i<MAX_PARTICLES; i++) {
        if(particles[i].active) {
            particles[i].position.x += particles[i].velocity.x * dt;
            particles[i].position.y += particles[i].velocity.y * dt;
            particles[i].life -= dt * 2.0f; // Fade out speed
            
            if(particles[i].life <= 0) {
                particles[i].active = false;
            }
        }
    }

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
    
    // --- INVENTORY INPUTS ---
    // Gun Switching
    if (IsKeyPressed(KEY_ONE)) player.inventory.currentGunIndex = 0;
    if (IsKeyPressed(KEY_TWO)) player.inventory.currentGunIndex = 1;
    if (IsKeyPressed(KEY_THREE)) player.inventory.currentGunIndex = 2;

    float wheelMove = GetMouseWheelMove();
    if (wheelMove != 0.0f) {
        if (wheelMove > 0) {
            player.inventory.currentGunIndex--;
            if (player.inventory.currentGunIndex < 0) player.inventory.currentGunIndex = 2;
        } else {
            player.inventory.currentGunIndex++;
            if (player.inventory.currentGunIndex > 2) player.inventory.currentGunIndex = 0;
        }
    }

    // Mask Switching
    if (IsKeyPressed(KEY_FOUR)) player.inventory.currentMaskIndex = 0;
    if (IsKeyPressed(KEY_FIVE)) player.inventory.currentMaskIndex = 1;
    if (IsKeyPressed(KEY_SIX)) player.inventory.currentMaskIndex = 2;

    Gun *currentGun = &player.inventory.gunSlots[player.inventory.currentGunIndex];
    Mask *currentMask = &player.inventory.maskSlots[player.inventory.currentMaskIndex];

    // Mask Interaction
    if (IsKeyPressed(KEY_C)) {
        if (currentMask->type != MASK_NONE) {
            currentMask->isActive = !currentMask->isActive;
        }
    }
    if (IsKeyPressed(KEY_G)) {
        if (currentMask->type != MASK_NONE) {
            currentMask->type = MASK_NONE;
            currentMask->isActive = false;
        }
    }
    
    bool hasGunEquipped = (currentGun->type != GUN_NONE);// && currentGun->type != GUN_KNIFE);

    // Player Update
    UpdatePlayer(&player, &currentLevel, dt, developerMode);

    // Map new inventory state to legacy renderer state
    PlayerEquipState currentEquipState = MapGunToEquip(currentGun->type);
    
    // Detect equipment changes for renderer
    if (currentEquipState != lastEquipmentState) {
        weaponShootTimer = 0.0f;
        PlayerRender_OnEquip(&playerRender, currentEquipState);
        lastEquipmentState = currentEquipState;
        
        // Auto-reload when switching to a weapon with 0 ammo
        if (currentGun->type != GUN_KNIFE && currentGun->type != GUN_NONE &&
            currentGun->currentAmmo == 0 && currentGun->reserveAmmo > 0) {
            player.isReloading = true;
            player.reloadTimer = currentGun->reloadTime;
        }
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
    PlayerRender_Update(&playerRender, &player, lastEquipmentState, dt, weaponShootTimer);
    
    // Handle reload timer
    if (player.isReloading) {
        player.reloadTimer -= dt;
        if (player.reloadTimer <= 0.0f) {
            // Reload complete
            int ammoNeeded = currentGun->maxAmmo - currentGun->currentAmmo;
            int ammoToTransfer = ammoNeeded < currentGun->reserveAmmo ? ammoNeeded : currentGun->reserveAmmo;
            currentGun->currentAmmo += ammoToTransfer;
            currentGun->reserveAmmo -= ammoToTransfer;
            player.isReloading = false;
            player.reloadTimer = 0.0f;
        }
    }
    
    // Manual reload with R key (only if not already reloading, has gun, and not full ammo)
    if (IsKeyPressed(KEY_R) && !player.isReloading && hasGunEquipped && 
        currentGun->type != GUN_KNIFE && currentGun->currentAmmo < currentGun->maxAmmo && 
        currentGun->reserveAmmo > 0) {
        player.isReloading = true;
        player.reloadTimer = currentGun->reloadTime;
        PlaySound(fxReload);
    }
    // --- LOGIC RESTORED ---

    // 1. Update Enemies
    for (int i = 0; i < currentLevel.enemyCount; i++) {
        UpdateEnemy(&currentLevel.enemies[i], player.position, &currentLevel, bullets, MAX_BULLETS, dt);
    }

    // 2. Player Shooting
    bool shootPressed = IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
    if (shootPressed) {
        bool canShoot = hasGunEquipped && !player.isReloading && weaponShootTimer <= 0;
        
        if (canShoot) {
            if (currentGun->currentAmmo > 0) {
                // Spawn Bullet
                for (int i = 0; i < MAX_BULLETS; i++) {
                    if (!bullets[i].active) {
                        bullets[i].active = true;
                        bullets[i].position = player.position;
                        bullets[i].radius = BULLET_RADIUS;
                        bullets[i].lifeTime = BULLET_LIFETIME;
                        bullets[i].isPlayerOwned = true;
                        bullets[i].velocity = Vector2Scale(aimDirNormalized, BULLET_SPEED);
                        bullets[i].damage = currentGun->damage; // Use gun damage stats
                        
                        currentGun->currentAmmo--;
                        weaponShootTimer = currentGun->cooldown;
                        PlaySound(fxShoot); // Ensure sound plays if loaded
                        break;
                    }
                }
                
                // Auto-reload check
                if (currentGun->currentAmmo == 0 && currentGun->reserveAmmo > 0) {
                     player.isReloading = true;
                     player.reloadTimer = currentGun->reloadTime;
                }
            } else {
                // Click empty, try reload
                if (currentGun->reserveAmmo > 0) {
                     player.isReloading = true;
                     player.reloadTimer = currentGun->reloadTime;
                }
            }
        } else if (currentGun->type == GUN_KNIFE && weaponShootTimer <= 0) {
             // Knife attack (visual only here, logic is melee below)
             weaponShootTimer = currentGun->cooldown; 
        }
    }

    // 3. Melee Logic (Knife / Choke / Stealth Kill)
    meleeTargetIndex = -1;
    
    // Check for Choke Target (Always check range for UI prompt)
    int chokeTarget = PlayerActions_GetClosestEnemyInRange(&currentLevel, player.position, 80.0f); // Increased range from 40->80
    if (chokeTarget != -1) {
        meleeTargetIndex = chokeTarget; // Set for UI prompt usage
    }

    // Choke Logic (Hold E)
    if (IsKeyDown(KEY_E)) {
         if (!player.isChoking) {
             // START CHOKING
             // 1. Find target
             int potentialTarget = PlayerActions_GetClosestEnemyInRange(&currentLevel, player.position, 80.0f);
             if (potentialTarget != -1) {
                 Entity *tgt = &currentLevel.enemies[potentialTarget];
                 // 2. Check visibility (Stealth)
                 bool seen = CheckLineOfSight(tgt, player.position, &currentLevel);
                 if (!seen && tgt->active) {
                     // 3. Start Choke
                     player.isChoking = true;
                     player.chokeTargetIndex = potentialTarget;
                     player.chokeTimer = 0.0f;
                     tgt->state = STATE_BEING_CHOKED;
                 }
             }
         } else {
             // CONTINUE CHOKING
             Entity *tgt = &currentLevel.enemies[player.chokeTargetIndex];
             if (tgt->active && tgt->state == STATE_BEING_CHOKED) {
                 player.chokeTimer += dt;
                 
                 // Lock positions (optional, or just disable movement inputs)
                 // Note: Input handling below should respect isChoking flag to disable movement.
                 
                 if (player.chokeTimer >= 1.0f) {
                     // KILL
                     PlayerActions_ApplyDamage(&currentLevel, player.chokeTargetIndex, 1000.0f, &player, droppedMasks, MAX_MASKS, droppedMaskRadius, droppedCards, MAX_CARDS, droppedGuns, MAX_DROPPED_GUNS);
                     player.isChoking = false;
                     // tgt state handled by ApplyDamage (likely inactive)
                 }
             } else {
                 // Target died or invalid
                 player.isChoking = false;
             }
         }
    } else {
        // RELEASED E
        if (player.isChoking) {
            // Cancel Choke
             Entity *tgt = &currentLevel.enemies[player.chokeTargetIndex];
             if (tgt->active && tgt->state == STATE_BEING_CHOKED) {
                 tgt->state = STATE_ATTACK; // Alerted!
             }
             player.isChoking = false;
             player.chokeTimer = 0.0f;
        }
    }
    
    // Knife Logic (Left Click if Knife is equipped)
    if (currentGun->type == GUN_KNIFE && IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && weaponShootTimer <= 0) {
         // Use the gun's range (should be 100.0f)
         int knifeTarget = PlayerActions_GetClosestEnemyInRange(&currentLevel, player.position, currentGun->range);
         if (knifeTarget != -1) {
             // Knife Damage = 50
             PlayerActions_ApplyDamage(&currentLevel, knifeTarget, currentGun->damage, &player, droppedMasks, MAX_MASKS, droppedMaskRadius, droppedCards, MAX_CARDS, droppedGuns, MAX_DROPPED_GUNS);
             PlaySound(fxShoot); // Just using shoot sound for now
         }
         weaponShootTimer = currentGun->cooldown;
    }
    
    // Legacy E key removal or remapping?
    // User said "not E anymore" for choking.
    // So removing E key logic block.

    // 4. Update Bullets
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (!bullets[i].active) continue;

        // Move
        bullets[i].position = Vector2Add(bullets[i].position, Vector2Scale(bullets[i].velocity, dt));
        bullets[i].lifeTime -= dt;

        bool hit = false;
        // Wall Collision
        for (int w = 0; w < currentLevel.wallCount; w++) {
            if (CheckCollisionCircleRec(bullets[i].position, bullets[i].radius, currentLevel.walls[w])) {
                hit = true; break;
            }
        }
        // Door Collision (Closed)
        if (!hit) {
            for (int d = 0; d < currentLevel.doorCount; d++) {
                if (!currentLevel.doors[d].isOpen &&
                    CheckCollisionCircleRec(bullets[i].position, bullets[i].radius, currentLevel.doors[d].rect)) {
                    hit = true; break;
                }
            }
        }

        if (hit || bullets[i].lifeTime <= 0) {
            bullets[i].active = false;
            continue;
        }

        // Entity Collision
        if (bullets[i].isPlayerOwned) {
            // Hit Enemy?
            for (int e = 0; e < currentLevel.enemyCount; e++) {
                if (currentLevel.enemies[e].active) {
                    if (CheckCollisionCircles(bullets[i].position, bullets[i].radius, currentLevel.enemies[e].position, currentLevel.enemies[e].radius)) {
                        // Kill Enemy -> DAMAGE
                        PlayerActions_ApplyDamage(&currentLevel, 
                                                e, 
                                                bullets[i].damage, // Use Bullet Damage
                                                &player, 
                                                droppedMasks, 
                                                MAX_MASKS, 
                                                droppedMaskRadius, 
                                                droppedCards, 
                                                MAX_CARDS, 
                                                droppedGuns, 
                                                MAX_DROPPED_GUNS);
                        
                        bullets[i].active = false;
                        break;
                    }
                }
            }
        } else {
            // Hit Player?
            if (CheckCollisionCircles(bullets[i].position, bullets[i].radius, player.position, player.radius)) {
                // Game Over Logic
                if (!developerMode) { // God mode check
                    player.health -= 1.0f;
                    SpawnBlood(player.position, 20); // SPLATTER!
                    
                    if (player.health <= 0.0f) {
                        gameOver = true; 
                    }
                    bullets[i].active = false;
                }
            }
        }
    }

    // 5. Mask Pickup
    for (int i = 0; i < MAX_MASKS; i++) {
        if (droppedMasks[i].active) {
            if (CheckCollisionCircles(player.position, player.radius, droppedMasks[i].position, droppedMasks[i].radius)) {
                if (IsKeyPressed(KEY_SPACE)) {
                    // Pickup logic: Find empty slot
                    int emptyIdx = -1;
                    for (int s = 0; s < MAX_MASK_SLOTS; s++) {
                        if (player.inventory.maskSlots[s].type == MASK_NONE) {
                            emptyIdx = s;
                            break;
                        }
                    }

                    if (emptyIdx != -1) {
                         // Map dropped identity to mask type
                         MaskAbilityType mType = MASK_SPEED; 
                         // Logic to determine type (random or based on enemy?)
                         if (droppedMasks[i].identity.color.r > 200) mType = MASK_STEALTH; // Red = Stealth?
                         
                         player.inventory.maskSlots[emptyIdx].type = mType;
                         player.inventory.maskSlots[emptyIdx].maxDuration = (mType == MASK_SPEED) ? 10.0f : 5.0f; 
                         player.inventory.maskSlots[emptyIdx].currentTimer = player.inventory.maskSlots[emptyIdx].maxDuration;
                         player.inventory.maskSlots[emptyIdx].isActive = false; 
                         player.inventory.maskSlots[emptyIdx].collected = true;
                         player.inventory.maskSlots[emptyIdx].color = droppedMasks[i].identity.color;
                         
                         droppedMasks[i].active = false;
                         DrawText("MASK EQUIPPED!", (int)player.position.x - 20, (int)player.position.y - 60, 10, GREEN);
                    } else {
                         DrawText("INVENTORY FULL! DROP MASK (G + Num)", (int)player.position.x - 50, (int)player.position.y - 60, 10, RED);
                    }
                }
            }
        }
    }
    
    // Drop Mask (Vanish)
    if (IsKeyPressed(KEY_G)) {
        // Drops the currently active or selected mask slot
        int currentMask = player.inventory.currentMaskIndex; // Use selected slot
        if (player.inventory.maskSlots[currentMask].type != MASK_NONE) {
             player.inventory.maskSlots[currentMask].type = MASK_NONE;
             player.inventory.maskSlots[currentMask].isActive = false;
             player.inventory.maskSlots[currentMask].collected = false;
             // Vanish - no entity spawned
        }
    }

    
    // 5.5 Card Pickup
    for (int i = 0; i < MAX_CARDS; i++) {
        if (droppedCards[i].active) {
            if (CheckCollisionCircles(player.position, player.radius, droppedCards[i].position, droppedCards[i].radius)) {
                // Determine if this card is better than what we have
                if (droppedCards[i].identity.permissionLevel > player.inventory.card.level) {
                     DrawText("PRESS SPACE TO PICKUP KEYCARD", (int)player.position.x - 50, (int)player.position.y - 40, 10, WHITE);
                     if (IsKeyPressed(KEY_SPACE)) {
                         player.inventory.card.level = droppedCards[i].identity.permissionLevel;
                         droppedCards[i].active = false;
                         // Play pickup sound?
                     }
                } else {
                    // Already have better or equal, maybe just auto-collect or ignore?
                    // Let's ignore for now but maybe show "ALREADY HAVE ACCESS"
                }
            }
        }
    }

    // 5.6 Gun Pickup
    for (int i = 0; i < MAX_DROPPED_GUNS; i++) {
        if (droppedGuns[i].active) {
            if (CheckCollisionCircles(player.position, player.radius, droppedGuns[i].position, droppedGuns[i].radius)) {
                DrawText("PRESS SPACE TO PICKUP GUN", (int)player.position.x - 50, (int)player.position.y - 40, 10, PINK);
                if (IsKeyPressed(KEY_SPACE)) {
                    // Try to find empty slot or a Knife slot to replace (SKIPPING SLOT 0)
                    int emptySlot = -1;
                    for (int s=1; s<MAX_GUN_SLOTS; s++) { // Start from 1
                        if (player.inventory.gunSlots[s].type == GUN_NONE || player.inventory.gunSlots[s].type == GUN_KNIFE) {
                            emptySlot = s;
                            break;
                        }
                    }
                    
                    if (emptySlot != -1) {
                        // Take it (Overwriting Knife/None in slot 1 or 2)
                        player.inventory.gunSlots[emptySlot] = droppedGuns[i].gun;
                        player.inventory.currentGunIndex = emptySlot; // Auto-switch?
                        droppedGuns[i].active = false;
                        PlaySound(fxReload); // Sound cue
                    } else {
                        // Swap with current if current is NOT Slot 0 and NOT a Knife
                        if (player.inventory.currentGunIndex > 0) {
                             Gun temp = player.inventory.gunSlots[player.inventory.currentGunIndex];
                             
                             if (temp.type != GUN_NONE && temp.type != GUN_KNIFE) {
                                player.inventory.gunSlots[player.inventory.currentGunIndex] = droppedGuns[i].gun;
                                
                                droppedGuns[i].gun = temp; // Swap data
                                droppedGuns[i].position = player.position; // Move drop to feet
                             } else {
                                 // Fallback (Shouldn't happen if emptySlot logic works)
                                 player.inventory.gunSlots[player.inventory.currentGunIndex] = droppedGuns[i].gun;
                                 droppedGuns[i].active = false;
                             }
                        } else {
                             // Holding Knife (Slot 0) and Slots 1/2 are full.
                             // Show UI "Inventory Full"? Or swap with Slot 1 by default?
                             // User requirement implies Slot 0 is safe. We just don't pick up.
                             DrawText("INVENTORY FULL - SWITCH WEAPON TO SWAP", (int)player.position.x - 60, (int)player.position.y - 50, 10, RED);
                        }
                    }
                }
            }
        }
    }
    
    // 5.7 Manual Gun Drop (Key Q)
    if (IsKeyPressed(KEY_Q)) {
        int idx = player.inventory.currentGunIndex;
        Gun current = player.inventory.gunSlots[idx];
        
        if (current.type != GUN_NONE && current.type != GUN_KNIFE) {
            // Find spot in droppedGuns
            for (int i=0; i<MAX_DROPPED_GUNS; i++) {
                if (!droppedGuns[i].active) {
                    droppedGuns[i].active = true;
                    droppedGuns[i].position = player.position;
                    // Offset slightly forward
                    droppedGuns[i].position.x += 20 * player.identity.speed * dt * cosf(player.rotation * DEG2RAD);
                    droppedGuns[i].position.y += 20 * player.identity.speed * dt * sinf(player.rotation * DEG2RAD);
                    
                    droppedGuns[i].radius = 15.0f;
                    droppedGuns[i].gun = current;
                    
                    // Revert slot to Knife
                    player.inventory.gunSlots[idx].type = GUN_KNIFE;
                    player.inventory.gunSlots[idx].active = true;
                    player.inventory.gunSlots[idx].damage = 50.0f;
                    player.inventory.gunSlots[idx].range = 100.0f;
                    player.inventory.gunSlots[idx].cooldown = 0.5f;
                    
                    break;
                }
            }
        }
    }

    // 6. Door Logic
    for (int i = 0; i < currentLevel.doorCount; i++) {
        PermissionLevel myLevel = player.inventory.card.level;
        
        bool sufficientPerm = myLevel >= currentLevel.doors[i].requiredPerm;
        if (developerMode) sufficientPerm = true; 
        
        if (CheckCollisionCircleRec(player.position, player.radius + 10.0f, currentLevel.doors[i].rect) && sufficientPerm) {
            currentLevel.doors[i].isOpen = true;
        } else {
            // Only close if player is far enough? Or auto close.
            // Simple auto - check if player is NOT in it.
            if (!CheckCollisionCircleRec(player.position, player.radius + 5.0f, currentLevel.doors[i].rect)) {
                 currentLevel.doors[i].isOpen = false;
            }
        }
        
        // Animate
        float speed = 2.0f; // Opening speed
        if (currentLevel.doors[i].isOpen) {
            currentLevel.doors[i].animationProgress += speed * dt;
            if (currentLevel.doors[i].animationProgress > 1.0f) currentLevel.doors[i].animationProgress = 1.0f;
        } else {
            currentLevel.doors[i].animationProgress -= speed * dt;
            if (currentLevel.doors[i].animationProgress < 0.0f) currentLevel.doors[i].animationProgress = 0.0f;
        }
    }

    // Check Win Condition (Reach Win Area or Enemies Cleared? or Zone 7?)
    // For Episode 1, let's say reach Zone 7 bottom? 
    if (player.position.y > 2000.0f) {
        gameWon = true;
    }

} // End UpdateGame

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
             DrawText("Press N to Next or R to Replay", sw / 2 - 150, sh / 2 + 30, 20, RAYWHITE);
        }
    } else {
        BeginMode2D(camera);

        for (int i = 0; i < currentLevel.bgs_count; i++) {
            Background* bg = &currentLevel.bgs[i];
            DrawTexturePro(bg->texture, bg->source, bg->dest, (Vector2){0,0}, 0.f, WHITE);
        }

        // Draw Level Elements
        if (playerDebugDraw) {
            for (int i = 0; i < currentLevel.wallCount; i++) DrawRectangleRec(currentLevel.walls[i], Fade(RED, 0.5f));
        }
        
        // Draw Doors (Sliding Sci-Fi Style)
        for (int i = 0; i < currentLevel.doorCount; i++) {
            Door *door = &currentLevel.doors[i];
            Color doorColor = SKYBLUE;
            if (door->requiredPerm == PERM_STAFF) doorColor = GREEN;
            else if (door->requiredPerm == PERM_GUARD) doorColor = RED;
            else if (door->requiredPerm == PERM_ADMIN) doorColor = PURPLE;
            
            // Determine orientation (Horizontal or Vertical) based on aspect ratio
            bool isHorizontal = door->rect.width > door->rect.height;
            
            float anim = door->animationProgress; // 0..1
            
            // Draw Door Panels
            if (isHorizontal) {
                // Splits Left/Right
                float w = door->rect.width / 2.0f;
                float slideDist = w * anim;
                
                Rectangle leftPanel = { door->rect.x - slideDist, door->rect.y, w, door->rect.height };
                Rectangle rightPanel = { door->rect.x + w + slideDist, door->rect.y, w, door->rect.height };
                
                // Draw Panels
                DrawRectangleRec(leftPanel, Fade(doorColor, 0.8f));
                DrawRectangleLinesEx(leftPanel, 2.0f, WHITE);
                
                DrawRectangleRec(rightPanel, Fade(doorColor, 0.8f));
                DrawRectangleLinesEx(rightPanel, 2.0f, WHITE);
                
            } else {
                // Splits Top/Bottom (Vertical Door)
                float h = door->rect.height / 2.0f;
                float slideDist = h * anim;
                
                Rectangle topPanel = { door->rect.x, door->rect.y - slideDist, door->rect.width, h };
                Rectangle bottomPanel = { door->rect.x, door->rect.y + h + slideDist, door->rect.width, h };
                
                 // Draw Panels
                DrawRectangleRec(topPanel, Fade(doorColor, 0.8f));
                DrawRectangleLinesEx(topPanel, 2.0f, WHITE);
                
                DrawRectangleRec(bottomPanel, Fade(doorColor, 0.8f));
                DrawRectangleLinesEx(bottomPanel, 2.0f, WHITE);
            }
            
            // Draw Lock Icon if closed
            if (anim < 0.1f) {
                Vector2 center = { door->rect.x + door->rect.width/2, door->rect.y + door->rect.height/2 };
                DrawCircleV(center, 4.0f, WHITE);
                if (door->requiredPerm > PERM_NONE) {
                     DrawRing(center, 6.0f, 8.0f, 0, 360, 0, doorColor);
                }
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
                
                // HP Bar
                float hpRatio = currentLevel.enemies[i].health / currentLevel.enemies[i].maxHealth;
                if (hpRatio < 0.0f) hpRatio = 0.0f;
                int barW = 40;
                int barH = 5;
                int barX = (int)currentLevel.enemies[i].position.x - barW/2;
                int barY = (int)currentLevel.enemies[i].position.y - 30;
                
                DrawRectangle(barX, barY, barW, barH, RED);
                DrawRectangle(barX, barY, (int)(barW * hpRatio), barH, GREEN);
                DrawRectangleLines(barX, barY, barW, barH, BLACK);
                
                // Text
                DrawText(TextFormat("%.0f", currentLevel.enemies[i].health), barX, barY - 10, 10, WHITE);
            }
        }

        // Mask
        for (int i = 0; i < MAX_MASKS; i++) {
            if (droppedMasks[i].active) {
                // Draw Striped Pattern
                Vector2 pos = droppedMasks[i].position;
                float r = droppedMasks[i].radius;
                Color col = droppedMasks[i].identity.color;
                
                DrawCircleV(pos, r, col);
                DrawCircleLines((int)pos.x, (int)pos.y, r, WHITE);
                
                // Stripes (Diagonal)
                rlPushMatrix();
                rlTranslatef(pos.x, pos.y, 0);
                rlRotatef(45.0f, 0, 0, 1);
                DrawRectangle(-r, -r/2, r*2, 4, WHITE);
                DrawRectangle(-r, 0, r*2, 4, WHITE);
                DrawRectangle(-r, r/2, r*2, 4, WHITE);
                rlPopMatrix();

                DrawText("MASK", (int)pos.x - 10, (int)pos.y - 10, 8, BLACK);
                DrawText("PRESS SPACE", (int)pos.x - 30, (int)pos.y - 30, 10, WHITE);
            }
        }

        // Cards
        for (int i = 0; i < MAX_CARDS; i++) {
            if (droppedCards[i].active) {
                // Draw a rectangle card
                Rectangle cardRect = { droppedCards[i].position.x - 8, droppedCards[i].position.y - 5, 16, 10 };
                DrawRectangleRec(cardRect, droppedCards[i].identity.color);
                DrawRectangleLinesEx(cardRect, 1, WHITE);
                DrawText("CARD", (int)droppedCards[i].position.x - 10, (int)droppedCards[i].position.y - 15, 8, WHITE);
            }
        }

        // Dropped Guns
        for (int i = 0; i < MAX_DROPPED_GUNS; i++) {
            if (droppedGuns[i].active) {
                // Determine text/color
                Color gunCol = ORANGE;
                const char* txt = "GUN";
                if (droppedGuns[i].gun.type == GUN_HANDGUN) { txt = "Pistol"; gunCol = GOLD; }
                else if (droppedGuns[i].gun.type == GUN_RIFLE) { txt = "Rifle"; gunCol = LIME; }
                
                DrawCircleV(droppedGuns[i].position, droppedGuns[i].radius, gunCol);
                DrawText(txt, (int)droppedGuns[i].position.x - 20, (int)droppedGuns[i].position.y - 20, 10, WHITE);
            }
        }

        // Bullets
        for (int i = 0; i < MAX_BULLETS; i++) {
            if (bullets[i].active) DrawCircleV(bullets[i].position, bullets[i].radius, bullets[i].isPlayerOwned ? YELLOW : ORANGE);
        }

        // Player
        if (playerRender.loaded) {
            PlayerRender_Draw(&playerRender, &player, lastEquipmentState);
            Gun *renderGun = &player.inventory.gunSlots[player.inventory.currentGunIndex];
            PlayerRender_DrawMuzzleFlash(&playerRender, &player, lastEquipmentState, weaponShootTimer, renderGun->cooldown);
        } else {
            PlayerRender_DrawFallback(player.position, player.radius); // Fallback if not loaded
        }

        // Melee Prompt
        if (meleeTargetIndex >= 0 && meleeTargetIndex < currentLevel.enemyCount && currentLevel.enemies[meleeTargetIndex].active) {
             DrawText("HOLD E TO CHOKE", (int)player.position.x - 40, (int)player.position.y - 60, 12, RED);
        }

        // Choking UI
        if (player.isChoking) {
             Vector2 center = player.position;
             float radius = 45.0f; 
             float progress = player.chokeTimer / 1.0f; 
             
             // Background
             DrawCircleLines((int)center.x, (int)center.y, radius, Fade(DARKGRAY, 0.8f));
             
             // Progress
             DrawCircleSector(center, radius, 0, 360 * progress, 36, Fade(RED, 0.7f));
             
             // Ring
             // DrawRing(center, innerRadius, outerRadius, startAngle, endAngle, segments, color);
             // DrawRing(center, innerRadius, outerRadius, startAngle, endAngle, segments, color);
             DrawRing(center, radius - 2, radius + 2, 0, 360 * progress, 36, RED);

             DrawText("CHOKING...", (int)center.x - 30, (int)center.y - 80, 10, RED);
        }
        
        // Draw Particles
        for(int i=0; i<MAX_PARTICLES; i++) {
            if(particles[i].active) {
                DrawRectangleV(particles[i].position, (Vector2){particles[i].size, particles[i].size}, Fade(particles[i].color, particles[i].life));
            }
        }
        
        // Debug
        if (playerDebugDraw) {
             DrawCircleLines((int)player.position.x, (int)player.position.y, player.radius, GOLD);
        }
        
        EndMode2D();
    // HUD
    Hud_DrawPlayer(&player);

    // UI: Active Mask Info
    int activeMaskIdx = -1;
    for(int i=0; i<3; i++) {
        if(player.inventory.maskSlots[i].isActive) {
             activeMaskIdx = i;
             break;
        }
    }
    
    if (activeMaskIdx != -1) {
        Mask m = player.inventory.maskSlots[activeMaskIdx];
        const char *maskName = (m.type == MASK_SPEED) ? "Speed Mask" : (m.type == MASK_STEALTH ? "Stealth Mask" : "Unknown Mask");
        const char *desc = (m.type == MASK_SPEED) ? "Ability: +50% Speed (10s)" : (m.type == MASK_STEALTH ? "Ability: Invisibility (5s)" : "Ability: None");
        
        DrawText(TextFormat("ACTIVE: %s (%.1fs)", maskName, m.currentTimer), GetScreenWidth()/2 - 150, 50, 24, GREEN);
        DrawText(desc, GetScreenWidth()/2 - 150, 80, 20, WHITE);
    } else {
        // Show selected slot info if valid
        Mask m = player.inventory.maskSlots[player.inventory.currentMaskIndex];
        if (m.type != MASK_NONE) {
             const char *maskName = (m.type == MASK_SPEED) ? "Speed Mask" : (m.type == MASK_STEALTH ? "Stealth Mask" : "Unknown Mask");
             DrawText(TextFormat("SELECTED: %s [PRESS C TO ACTIVATE]", maskName), GetScreenWidth()/2 - 200, 50, 20, YELLOW);
        }
    }

    if (levelStartTimer > 0) {
        DrawText("READY...", GetScreenWidth()/2 - 50, GetScreenHeight()/2, 30, RED);
    }
    
    if (developerMode) {
        DrawText("DEV MODE ON (F) - GOD & UNLOCK", 10, 10, 20, GREEN);
    }
    }

	level_editor_draw(&editor, &camera);
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
}

