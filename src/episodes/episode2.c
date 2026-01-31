#include "../enemies/enemy.h"
#include "../types.h"
#include "episodes.h"

#define ENEMY_SHOOT_INTERVAL 1.5f

void InitEpisode2(Level *level) {
    level->id = 2;
    level->playerSpawn = (Vector2){100.0f, 360.0f};
    level->playerStartId = GetIdentity(ENEMY_CIVILIAN);
    level->winX = 6100.0f; // Much longer level

    level->wallCount = 0;
    level->doorCount = 0;
    level->enemyCount = 0;

    // --- WALLS ---
    // Boundaries
    level->walls[level->wallCount++] = (Rectangle){-50, -50, 7100, 50}; // Top
    level->walls[level->wallCount++] = (Rectangle){-50, 720, 7100, 50}; // Bottom
    level->walls[level->wallCount++] = (Rectangle){-50, 0, 50, 720};    // Start
    level->walls[level->wallCount++] = (Rectangle){7000, 0, 50, 720};   // End

    // Zone 1: Reception / Waiting Area (Civilian -> Staff)
    // Lots of pillars/seats
    for(int i=0; i<5; i++) {
        level->walls[level->wallCount++] = (Rectangle){400 + i*200, 200, 50, 50};
        level->walls[level->wallCount++] = (Rectangle){400 + i*200, 500, 50, 50};
    }
    // Gate to Zone 2
    level->walls[level->wallCount++] = (Rectangle){1500, 0, 50, 300};
    level->walls[level->wallCount++] = (Rectangle){1500, 420, 50, 300};
    level->doors[level->doorCount] = (Rectangle){1500, 300, 50, 120};
    level->doorPerms[level->doorCount] = PERM_STAFF;
    level->doorsOpen[level->doorCount] = false;
    level->doorCount++;

    // Zone 2: Office Cubicles (Staff -> Guard)
    // Maze-like cubicle walls
    for(int i=0; i<3; i++) {
        level->walls[level->wallCount++] = (Rectangle){1800 + i*400, 150, 20, 200};
        level->walls[level->wallCount++] = (Rectangle){2000 + i*400, 400, 20, 200};
    }
    // Gate to Zone 3
    level->walls[level->wallCount++] = (Rectangle){3000, 0, 50, 300};
    level->walls[level->wallCount++] = (Rectangle){3000, 420, 50, 300};
    level->doors[level->doorCount] = (Rectangle){3000, 300, 50, 120};
    level->doorPerms[level->doorCount] = PERM_GUARD;
    level->doorsOpen[level->doorCount] = false;
    level->doorCount++;

    // Zone 3: High Security / Server Room (Guard -> Admin)
    // Long corridors with cover
    level->walls[level->wallCount++] = (Rectangle){3200, 200, 500, 20};
    level->walls[level->wallCount++] = (Rectangle){3800, 500, 500, 20};
    level->walls[level->wallCount++] = (Rectangle){4000, 200, 20, 200};
    
    // Gate to Zone 4
    level->walls[level->wallCount++] = (Rectangle){4500, 0, 50, 300};
    level->walls[level->wallCount++] = (Rectangle){4500, 420, 50, 300};
    level->doors[level->doorCount] = (Rectangle){4500, 300, 50, 120};
    level->doorPerms[level->doorCount] = PERM_ADMIN;
    level->doorsOpen[level->doorCount] = false;
    level->doorCount++;

    // Zone 4: Executive / Admin Area (Admin)
    // Open arena with pillars
    level->walls[level->wallCount++] = (Rectangle){4800, 150, 50, 50};
    level->walls[level->wallCount++] = (Rectangle){4800, 550, 50, 50};
    level->walls[level->wallCount++] = (Rectangle){5500, 150, 50, 50};
    level->walls[level->wallCount++] = (Rectangle){5500, 550, 50, 50};
    level->walls[level->wallCount++] = (Rectangle){5150, 335, 50, 50}; // Center pillar

    // --- ENEMIES ---
    // Zone 1 (Staff needed) - Civilans/Staff mixed
    level->enemies[level->enemyCount++] = InitEnemy((Vector2){600, 250}, ENEMY_CIVILIAN);
    level->enemies[level->enemyCount++] = InitEnemy((Vector2){600, 450}, ENEMY_CIVILIAN);
    level->enemies[level->enemyCount++] = InitEnemy((Vector2){1000, 360}, ENEMY_STAFF); // Key holder
    level->enemies[level->enemyCount++] = InitEnemy((Vector2){1200, 150}, ENEMY_CIVILIAN);
    level->enemies[level->enemyCount++] = InitEnemy((Vector2){1200, 550}, ENEMY_STAFF);

    // Zone 2 (Guard needed) - Staff/Guards
    level->enemies[level->enemyCount++] = InitEnemy((Vector2){1700, 100}, ENEMY_STAFF);
    level->enemies[level->enemyCount++] = InitEnemy((Vector2){1900, 600}, ENEMY_STAFF);
    level->enemies[level->enemyCount++] = InitEnemy((Vector2){2200, 300}, ENEMY_GUARD); // Key holder
    level->enemies[level->enemyCount++] = InitEnemy((Vector2){2400, 100}, ENEMY_GUARD);
    level->enemies[level->enemyCount++] = InitEnemy((Vector2){2600, 600}, ENEMY_STAFF);
    level->enemies[level->enemyCount++] = InitEnemy((Vector2){2800, 360}, ENEMY_GUARD);

    // Zone 3 (Admin needed) - Guards/Admins
    level->enemies[level->enemyCount++] = InitEnemy((Vector2){3200, 100}, ENEMY_GUARD);
    level->enemies[level->enemyCount++] = InitEnemy((Vector2){3200, 600}, ENEMY_GUARD);
    level->enemies[level->enemyCount++] = InitEnemy((Vector2){3600, 360}, ENEMY_ADMIN); // Key holder
    level->enemies[level->enemyCount++] = InitEnemy((Vector2){3800, 100}, ENEMY_GUARD);
    level->enemies[level->enemyCount++] = InitEnemy((Vector2){4000, 600}, ENEMY_GUARD);
    level->enemies[level->enemyCount++] = InitEnemy((Vector2){4300, 360}, ENEMY_GUARD);

    // Zone 4 (End) - Admins/Elite
    level->enemies[level->enemyCount++] = InitEnemy((Vector2){4700, 360}, ENEMY_ADMIN);
    level->enemies[level->enemyCount++] = InitEnemy((Vector2){5000, 200}, ENEMY_ADMIN);
    level->enemies[level->enemyCount++] = InitEnemy((Vector2){5000, 500}, ENEMY_ADMIN);
    level->enemies[level->enemyCount++] = InitEnemy((Vector2){5500, 100}, ENEMY_ADMIN);
    level->enemies[level->enemyCount++] = InitEnemy((Vector2){5500, 600}, ENEMY_ADMIN);
    level->enemies[level->enemyCount++] = InitEnemy((Vector2){6000, 360}, ENEMY_ADMIN); // Final boss
}

void UnloadEpisode2() {

}
