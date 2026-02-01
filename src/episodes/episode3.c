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
    level->bgs_count = 1;
    static Texture2D level_bg_textures; // Cache
    
    // Load if needed (Lazy load safety)
    if (level_bg_textures.id == 0) level_bg_textures = LoadTexture("assets/environment/background_3_1.png"); // Concrete

    // Zone 1: Lobby (Tiles)
    level->bgs[0] = (Background){level_bg_textures, (Rectangle){0,0,8092,8092}, (Rectangle){0,0,8092,8092}};

    // --- WALLS (Maze Layout) ---
    level->wallCount = 0;

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
