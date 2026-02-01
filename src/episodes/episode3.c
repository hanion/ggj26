#include "../enemies/enemy.h"
#include "../types.h"
#include "episodes.h"
#include <stdio.h> // For getting NULL

void InitEpisode3(Level *level) {
    level->id = 3; // Episode 3
    level->playerSpawn = (Vector2){100.0f, 300.0f};
    level->playerStartId = GetIdentity(ENEMY_CIVILIAN); // Start undercover
    level->winX = 3900.0f; // Challenge: Reach the far right
    
    // --- TEXTURES (10 Backgrounds as requested) ---
    // We only have ~7 distinct files, so we reuse them to create 10 distinct flooring zones
    level->bgs_count = 10;
    static Texture2D level_bg_textures[8]; // Cache
    
    // Load if needed (Lazy load safety)
    if (level_bg_textures[0].id == 0) level_bg_textures[0] = LoadTexture("assets/environment/background_1.png"); // Concrete
    if (level_bg_textures[1].id == 0) level_bg_textures[1] = LoadTexture("assets/environment/background_2.png"); // Tiles
    if (level_bg_textures[2].id == 0) level_bg_textures[2] = LoadTexture("assets/environment/background_3.png"); // Office
    if (level_bg_textures[3].id == 0) level_bg_textures[3] = LoadTexture("assets/environment/background_4.png"); // Dark
    if (level_bg_textures[4].id == 0) level_bg_textures[4] = LoadTexture("assets/environment/background_5.png"); // Lab
    if (level_bg_textures[5].id == 0) level_bg_textures[5] = LoadTexture("assets/environment/background_6.png"); // Server
    if (level_bg_textures[6].id == 0) level_bg_textures[6] = LoadTexture("assets/environment/background_7.png"); // Vault

    // Zone 1: Lobby (Tiles)
    level->bgs[0] = (Background){level_bg_textures[1], (Rectangle){0,0,800,800}, (Rectangle){0,0,800,800}};
    level->bgs[1] = (Background){level_bg_textures[1], (Rectangle){0,0,800,800}, (Rectangle){0,800,800,800}};
    
    // Zone 2: Corridors (Concrete) - Hard transition
    level->bgs[2] = (Background){level_bg_textures[0], (Rectangle){0,0,800,200}, (Rectangle){800, 300, 1000, 200}};
    level->bgs[3] = (Background){level_bg_textures[0], (Rectangle){0,0,200,800}, (Rectangle){1600, 0, 200, 1600}};
    
    // Zone 3: Offices (Office + Dark)
    level->bgs[4] = (Background){level_bg_textures[2], (Rectangle){0,0,800,600}, (Rectangle){1800, 0, 800, 600}};
    level->bgs[5] = (Background){level_bg_textures[3], (Rectangle){0,0,800,600}, (Rectangle){1800, 800, 800, 600}};
    
    // Zone 4: Server Room
    level->bgs[6] = (Background){level_bg_textures[5], (Rectangle){0,0,600,600}, (Rectangle){2600, 200, 600, 1000}};
    
    // Zone 5: Lab
    level->bgs[7] = (Background){level_bg_textures[4], (Rectangle){0,0,500,500}, (Rectangle){3200, 0, 600, 700}};
    
    // Zone 6: Vault (Final)
    level->bgs[8] = (Background){level_bg_textures[6], (Rectangle){0,0,400,800}, (Rectangle){3200, 800, 800, 800}};

    // Extra decoration
    level->bgs[9] = (Background){level_bg_textures[0], (Rectangle){0,0,100,100}, (Rectangle){-200, 0, 200, 1600}}; // Left border

    // --- WALLS (Maze Layout) ---
    level->wallCount = 0;
    
    // Boundaries
    level->walls[level->wallCount++] = (Wall){(Rectangle){-50, -50, 4200, 50}, 0.0f}; // Top
    level->walls[level->wallCount++] = (Wall){(Rectangle){-50, 1600, 4200, 50}, 0.0f}; // Bottom
    level->walls[level->wallCount++] = (Wall){(Rectangle){-50, 0, 50, 1650}, 0.0f}; // Left
    level->walls[level->wallCount++] = (Wall){(Rectangle){4000, 0, 50, 1650}, 0.0f}; // Right
    
    // Lobby dividers
    level->walls[level->wallCount++] = (Wall){(Rectangle){800, 0, 20, 300}, 0.0f}; 
    level->walls[level->wallCount++] = (Wall){(Rectangle){800, 500, 20, 1100}, 0.0f}; 
    
    // Corridor Walls
    level->walls[level->wallCount++] = (Wall){(Rectangle){1000, 300, 20, 600}, 0.0f}; // Force zigzag
    level->walls[level->wallCount++] = (Wall){(Rectangle){1400, 500, 20, 1100}, 0.0f}; 
    
    // Office Rooms
    level->walls[level->wallCount++] = (Wall){(Rectangle){1800, 600, 800, 20}, 0.0f}; // Middle divider
    
    // Security Chokepoint
    level->walls[level->wallCount++] = (Wall){(Rectangle){2600, 0, 20, 200}, 0.0f};
    level->walls[level->wallCount++] = (Wall){(Rectangle){2600, 400, 20, 1200}, 0.0f};
    
    // Vault protection
    level->walls[level->wallCount++] = (Wall){(Rectangle){3200, 0, 20, 700}, 0.0f};
    level->walls[level->wallCount++] = (Wall){(Rectangle){3200, 900, 20, 700}, 0.0f};

    // --- DOORS ---
    level->doorCount = 0;
    
    // 1. Lobby Exit (Requires ANY Mask/Card - Level 0 is fine, but blocks vision)
    level->doors[level->doorCount++] = (Door){ (Rectangle){800, 300, 20, 200}, PERM_NONE, false, 0.0f };
    
    // 2. Security Check (Staff)
    level->doors[level->doorCount++] = (Door){ (Rectangle){1400, 300, 20, 200}, PERM_STAFF, false, 0.0f };
    
    // 3. Office Entry (Guard)
    level->doors[level->doorCount++] = (Door){ (Rectangle){1600, 600, 200, 20}, PERM_GUARD, false, 0.0f };
    
    // 4. Server Room (Guard)
    level->doors[level->doorCount++] = (Door){ (Rectangle){2600, 200, 20, 200}, PERM_GUARD, false, 0.0f };
    
    // 5. Vault Entry (Admin - RED)
    level->doors[level->doorCount++] = (Door){ (Rectangle){3200, 700, 20, 200}, PERM_ADMIN, false, 0.0f };


    // --- ENEMIES (Hard Difficulty) ---
    level->enemyCount = 0;
    
    // Lobby: Civilians + 1 Watcher
    level->enemies[level->enemyCount++] = InitEnemy((Vector2){400, 400}, ENEMY_CIVILIAN);
    level->enemies[level->enemyCount++] = InitEnemy((Vector2){600, 200}, ENEMY_CIVILIAN);
    level->enemies[level->enemyCount++] = InitEnemy((Vector2){600, 1200}, ENEMY_STAFF); // Has card
    
    // Corridor: Patrolling Guards (Armed)
    level->enemies[level->enemyCount++] = InitEnemy((Vector2){1200, 400}, ENEMY_GUARD); 
    level->enemies[level->enemyCount].inventory.gunSlots[0].type = GUN_HANDGUN; // Give Gun
    level->enemies[level->enemyCount++].identity.color = RED;
    
    level->enemies[level->enemyCount++] = InitEnemy((Vector2){1500, 1400}, ENEMY_GUARD);
    
    // Office: Mixed Bag
    level->enemies[level->enemyCount++] = InitEnemy((Vector2){2000, 300}, ENEMY_STAFF);
    level->enemies[level->enemyCount++] = InitEnemy((Vector2){2200, 1000}, ENEMY_GUARD); // Key holder?
    
    // Server Room: Elite Guards (Rifles if implemented, else Shotgun/Handgun)
    level->enemies[level->enemyCount++] = InitEnemy((Vector2){2900, 600}, ENEMY_ADMIN); // Has Admin Card
    level->enemies[level->enemyCount].inventory.gunSlots[0].type = GUN_RIFLE;
    level->enemies[level->enemyCount++].identity.speed *= 1.2f; // Fast
    
    // Vault: The Bosses
    level->enemies[level->enemyCount++] = InitEnemy((Vector2){3500, 1200}, ENEMY_ADMIN);
    level->enemies[level->enemyCount++] = InitEnemy((Vector2){3600, 1400}, ENEMY_ADMIN);
    level->enemies[level->enemyCount++] = InitEnemy((Vector2){3800, 1300}, ENEMY_GUARD);
    
    // Ambushers (Hidden in corners)
    level->enemies[level->enemyCount++] = InitEnemy((Vector2){2500, 50}, ENEMY_GUARD);

}

void UnloadEpisode3() {
    // Unload textures if manageable? 
    // Raylib caches usually, but for now we keep it simple.
}
