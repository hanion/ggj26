#include "../enemies/enemy.h"
#include "../types.h"
#include "episodes.h"
#include <stdio.h>

void InitEpisode2(Level *level) {
    level->id = 2;
    level->playerSpawn = (Vector2){200.0f, 600.0f}; // Start in BG 1
    level->playerStartId = GetIdentity(ENEMY_GUARD);
    level->winX = 8000.0f; // Long level
    
    // --- TEXTURES (New Assets) ---
    level->bgs_count = 5;
    static Texture2D bg1, bg2, bg3, bg4, bg5;
    
    if (bg1.id == 0) bg1 = LoadTexture("assets/environment/new_backg_1.png");
    if (bg2.id == 0) bg2 = LoadTexture("assets/environment/new_backg_2.png");
    if (bg3.id == 0) bg3 = LoadTexture("assets/environment/new_backg_3.png");
    if (bg4.id == 0) bg4 = LoadTexture("assets/environment/new_backg_4.png");
    if (bg5.id == 0) bg5 = LoadTexture("assets/environment/new_backg_5.png");
    
    // Layout: Cluster ("Complicated")
    // BG2 (Hall) is Central. width approx 1000-1500?
    // Let's assume average 1000 for calculation, user can adjust in Editor if overlaps occur.
    
    // 2. Central Hall
    level->bgs[1] = (Background){bg2, (Rectangle){0,0,bg2.width,bg2.height}, (Rectangle){0, 0, bg2.width, bg2.height}};
    
    // 1. Kitchen (Left of Hall)
    level->bgs[0] = (Background){bg1, (Rectangle){0,0,bg1.width,bg1.height}, (Rectangle){-bg1.width, 0, bg1.width, bg1.height}};
    
    // 3. Blue Room (Right of Hall)
    level->bgs[2] = (Background){bg3, (Rectangle){0,0,bg3.width,bg3.height}, (Rectangle){bg2.width, 0, bg3.width, bg3.height}};
    
    // 4. Bathroom (Above Blue Room)
    level->bgs[3] = (Background){bg4, (Rectangle){0,0,bg4.width,bg4.height}, (Rectangle){bg2.width, -bg4.height, bg4.width, bg4.height}};
    
    // 5. Living Room (Right of Blue Room)
    level->bgs[4] = (Background){bg5, (Rectangle){0,0,bg5.width,bg5.height}, (Rectangle){bg2.width + bg3.width, 0, bg5.width, bg5.height}};
    
    level->winX = 8000.0f; // Far right

    // --- WALLS ---
    // Cleared for Editor use
    level->wallCount = 0;
    
    // --- DOORS ---
    level->doorCount = 0;
    
    // --- WALLS ---
    level->wallCount = 0;
    
    // 2. Central Hall Walls (Rectangle{0, 0, bg2.width, bg2.height})
    level->walls[level->wallCount++] = (Wall){(Rectangle){0, 0, bg2.width, 50}, 0.0f}; // Top
    level->walls[level->wallCount++] = (Wall){(Rectangle){0, bg2.height-50, bg2.width, 50}, 0.0f}; // Bottom (Partial? Leave gap for rooms if needed, but for now full)
    
    // --- VERTICAL DIVIDERS (With Gaps/Doorways) ---
    float doorSize = 250.0f; // Wide enough for player
    
    // 1. Kitchen <-> Hall [Divider at x=0]
    // Top segment
    level->walls[level->wallCount++] = (Wall){(Rectangle){-20, 0, 20, bg1.height/2 - doorSize/2}, 0.0f};
    // Bottom segment
    level->walls[level->wallCount++] = (Wall){(Rectangle){-20, bg1.height/2 + doorSize/2, 20, bg1.height/2 - doorSize/2}, 0.0f};
    
    // Gap Filler (Hall is taller than Kitchen)
    if (bg2.height > bg1.height) {
        level->walls[level->wallCount++] = (Wall){(Rectangle){-20, bg1.height, 20, bg2.height - bg1.height}, 0.0f};
    }

    // 2. Hall <-> Blue Room [Divider at x=bg2.width]
    // Top segment
    level->walls[level->wallCount++] = (Wall){(Rectangle){bg2.width, 0, 20, bg2.height/2 - doorSize/2}, 0.0f}; 
    // Bottom segment
    level->walls[level->wallCount++] = (Wall){(Rectangle){bg2.width, bg2.height/2 + doorSize/2, 20, bg2.height/2 - doorSize/2}, 0.0f};

    // 3. Blue Room <-> Living Room [Divider at x=bg2.width + bg3.width]
    float xLivingStart = bg2.width + bg3.width;
    // Top segment
    level->walls[level->wallCount++] = (Wall){(Rectangle){xLivingStart-20, 0, 20, bg3.height/2 - doorSize/2}, 0.0f};
    // Bottom segment
    level->walls[level->wallCount++] = (Wall){(Rectangle){xLivingStart-20, bg3.height/2 + doorSize/2, 20, bg3.height/2 - doorSize/2}, 0.0f};
    
    // 4. Blue Room <-> Bathroom [Divider at top of Blue Room, y=0]
    // Left segment
    float xBathStart = bg2.width;
    level->walls[level->wallCount++] = (Wall){(Rectangle){xBathStart, 0, bg3.width/2 - doorSize/2, 20}, 0.0f};
    // Right segment
    level->walls[level->wallCount++] = (Wall){(Rectangle){xBathStart + bg3.width/2 + doorSize/2, 0, bg3.width/2 - doorSize/2, 20}, 0.0f};

    // ---- DOORS ----
    level->doorCount = 4;
    level->doors[0].rect = (Rectangle){-20.000000,216.000000,20.000000,250.000000};
    level->doors[0].requiredPerm = PERM_STAFF;
    level->doors[1].rect = (Rectangle){1371.000000,360.000000,20.000000,250.000000};
    level->doors[1].requiredPerm = PERM_GUARD;
    level->doors[2].rect = (Rectangle){2369.000000,214.000000,20.000000,250.000000};
    level->doors[2].requiredPerm = PERM_ADMIN;
    level->doors[3].rect = (Rectangle){1755.000000,0.000000,250.000000,20.000000};
    level->doors[3].requiredPerm = PERM_STAFF;

    // 1. Kitchen Walls (Rectangle{-bg1.width, 0, bg1.width, bg1.height})
    level->walls[level->wallCount++] = (Wall){(Rectangle){-bg1.width, 0, bg1.width, 50}, 0.0f}; // Top
    level->walls[level->wallCount++] = (Wall){(Rectangle){-bg1.width, bg1.height-50, bg1.width, 50}, 0.0f}; // Bottom
    level->walls[level->wallCount++] = (Wall){(Rectangle){-bg1.width, 0, 50, bg1.height}, 0.0f}; // Far Left Wall
    
    // 3. Blue Room Walls (Rectangle{bg2.width, 0, bg3.width, bg3.height})
    float xBlue = bg2.width;
    // REMOVED TOP WALL FOR BATHROOM ACCESS
    level->walls[level->wallCount++] = (Wall){(Rectangle){xBlue, bg3.height-50, bg3.width, 50}, 0.0f}; // Bottom
    
    // 4. Bathroom Walls (Rectangle{bg2.width, -bg4.height, bg4.width, bg4.height})
    float xBath = bg2.width;
    float yBath = -bg4.height;
    level->walls[level->wallCount++] = (Wall){(Rectangle){xBath, yBath, bg4.width, 50}, 0.0f}; // Top
    level->walls[level->wallCount++] = (Wall){(Rectangle){xBath, yBath, 50, bg4.height}, 0.0f}; // Left
    level->walls[level->wallCount++] = (Wall){(Rectangle){xBath + bg4.width - 50, yBath, 50, bg4.height}, 0.0f}; // Right
    
    // 5. Living Room Walls (Rectangle{bg2.width + bg3.width, 0, bg5.width, bg5.height})
    float xLiving = bg2.width + bg3.width;
    level->walls[level->wallCount++] = (Wall){(Rectangle){xLiving, 0, bg5.width, 50}, 0.0f}; // Top
    level->walls[level->wallCount++] = (Wall){(Rectangle){xLiving, bg5.height-50, bg5.width, 50}, 0.0f}; // Bottom
    level->walls[level->wallCount++] = (Wall){(Rectangle){xLiving + bg5.width - 50, 0, 50, bg5.height}, 0.0f}; // Far Right Wall

    // --- ENEMIES (Key Droppers) ---
    level->enemyCount = 5;
    
    // 1. Kitchen Staff (Has Staff Card for Kitchen Door? No, Player is inside Kitchen maybe? Or Hall?)
    // Let's place a Staff in the Hall to open Kitchen? Or Kitchen Staff to open Bathroom?
    // Player spawns in Hall (bg2).
    level->playerSpawn = (Vector2){200.0f, 600.0f}; // Actually bg2 starts at x=0. 200 is inside Hall.
    
    // Enemy 1: Hall Staff (Drops STAFF card -> Opens Kitchen/Bathroom)
    level->enemies[0] = InitEnemy((Vector2){500.0f, 600.0f}, ENEMY_STAFF);
    
    // Enemy 2: Kitchen Guard (Drops GUARD card -> Opens Blue Room)
    level->enemies[1] = InitEnemy((Vector2){-500.0f, 400.0f}, ENEMY_GUARD);
    
    // Enemy 3: Blue Room Guard (Drops nothing/Guard)
    level->enemies[2] = InitEnemy((Vector2){bg2.width + 300.0f, 400.0f}, ENEMY_GUARD);
    
    // Enemy 4: Bathroom Admin (Drops ADMIN card -> Opens Living Room)
    level->enemies[3] = InitEnemy((Vector2){bg2.width + 200.0f, -300.0f}, ENEMY_ADMIN);
    
    // Enemy 5: Living Room Boss
    level->enemies[4] = InitEnemy((Vector2){xLivingStart + 400.0f, 400.0f}, ENEMY_ADMIN);
}

void UnloadEpisode2() {
    // Unload if unique textures were loaded
}
