#include "../enemies/enemy.h"
#include "../types.h"
#include "episodes.h"

static Texture2D texZone1;
static Texture2D texZone2;
static Texture2D texZone3;
static Texture2D texZone4;

#define ENEMY_SHOOT_INTERVAL 1.5f

void InitEpisode2(Level *level) {
    level->id = 2;
    level->playerSpawn = (Vector2){100.0f, 320.0f};
    level->playerStartId = GetIdentity(ENEMY_CIVILIAN);
    level->winX = 3800.0f; // Adjusted for 4 zones

    level->wallCount = 0;
    level->doorCount = 0;
    level->enemyCount = 0;
    level->bgs_count = 0;

    // Load Textures (Reusing mostly valid ones)
    if (texZone1.id == 0)texZone1 = LoadTexture("assets/environment/background_1.png"); // Reception
    if (texZone2.id == 0)texZone2 = LoadTexture("assets/environment/background_2.png"); // Office
    if (texZone3.id == 0)texZone3 = LoadTexture("assets/environment/background_6.png"); // Security/Server
    if (texZone4.id == 0)texZone4 = LoadTexture("assets/environment/background_3.png"); // Executive

    // --- BACKGROUNDS ---
    // Zone 1: Reception (Start)
    // Size: 913x642
    level->bgs[level->bgs_count++] = (Background){texZone1, (Rectangle){0,0,texZone1.width,texZone1.height}, (Rectangle){0,0,913,642}};
    
    // Zone 2: Office (Right of Z1)
    // Size: 911x661 -> Placed at X=913
    level->bgs[level->bgs_count++] = (Background){texZone2, (Rectangle){0,0,texZone2.width,texZone2.height}, (Rectangle){913,0,911,661}};

    // Zone 3: Security (Right of Z2)
    // Size: 795x853 -> Placed at X=1824
    level->bgs[level->bgs_count++] = (Background){texZone3, (Rectangle){0,0,texZone3.width,texZone3.height}, (Rectangle){1824,0,795,853}};

    // Zone 4: Executive (Right of Z3)
    // Size: 957x654 -> Placed at X=2619
    level->bgs[level->bgs_count++] = (Background){texZone4, (Rectangle){0,0,texZone4.width,texZone4.height}, (Rectangle){2619,0,957,654}};


    // --- WALLS & DOORS ---
    
    // -- Zone 1 (0..913, 0..642) --
    // Top/Bottom
    level->walls[level->wallCount++] = (Wall){(Rectangle){0, -50, 913, 50}, 0.0f}; // Top
    level->walls[level->wallCount++] = (Wall){(Rectangle){0, 642, 913, 50}, 0.0f}; // Bottom
    level->walls[level->wallCount++] = (Wall){(Rectangle){-50, 0, 50, 642}, 0.0f}; // Start
    
    // Pillars/Decor in Z1
    level->walls[level->wallCount++] = (Wall){(Rectangle){300, 200, 40, 40}, 0.0f};
    level->walls[level->wallCount++] = (Wall){(Rectangle){300, 400, 40, 40}, 0.0f};
    level->walls[level->wallCount++] = (Wall){(Rectangle){600, 200, 40, 40}, 0.0f};
    level->walls[level->wallCount++] = (Wall){(Rectangle){600, 400, 40, 40}, 0.0f};

    // Door to Z2 (at 913)
    level->walls[level->wallCount++] = (Wall){(Rectangle){913, 0, 20, 260}, 0.0f};
    level->walls[level->wallCount++] = (Wall){(Rectangle){913, 380, 20, 300}, 0.0f};
    level->doors[level->doorCount].rect = (Rectangle){913, 260, 20, 120};
    level->doors[level->doorCount].requiredPerm = PERM_STAFF;
    level->doors[level->doorCount].isOpen = false;
    level->doors[level->doorCount].animationProgress = 0.0f;
    level->doorCount++;


    // -- Zone 2 (913..1824, 0..661) --
    // Top/Bottom
    level->walls[level->wallCount++] = (Wall){(Rectangle){913, -50, 911, 50}, 0.0f};
    level->walls[level->wallCount++] = (Wall){(Rectangle){913, 661, 911, 50}, 0.0f}; 
    
    // Office Cubicles
    for(int i=0; i<3; i++) {
        level->walls[level->wallCount++] = (Wall){(Rectangle){1200 + i*200, 150, 10, 200}, 0.0f};
        level->walls[level->wallCount++] = (Wall){(Rectangle){1300 + i*200, 400, 10, 200}, 0.0f};
    }

    // Door to Z3 (at 1824)
    level->walls[level->wallCount++] = (Wall){(Rectangle){1824, 0, 20, 260}, 0.0f};
    level->walls[level->wallCount++] = (Wall){(Rectangle){1824, 380, 20, 500}, 0.0f}; 
    // Z2 ends at Y=661. Z3 starts Y=0 ends Y=853.
    // So the gap is below 661. But Z2 has a wall at 661. So it's fine.
    
    level->doors[level->doorCount].rect = (Rectangle){1824, 260, 20, 120};
    level->doors[level->doorCount].requiredPerm = PERM_GUARD;
    level->doors[level->doorCount].isOpen = false;
    level->doors[level->doorCount].animationProgress = 0.0f;
    level->doorCount++;


    // -- Zone 3 (1824..2619, 0..853) -- Security
    // Top/Bottom
    level->walls[level->wallCount++] = (Wall){(Rectangle){1824, -50, 795, 50}, 0.0f};
    level->walls[level->wallCount++] = (Wall){(Rectangle){1824, 853, 795, 50}, 0.0f};

    // Server Racks?
    level->walls[level->wallCount++] = (Wall){(Rectangle){2000, 200, 20, 400}, 0.0f};
    level->walls[level->wallCount++] = (Wall){(Rectangle){2200, 200, 20, 400}, 0.0f};
    level->walls[level->wallCount++] = (Wall){(Rectangle){2400, 200, 20, 400}, 0.0f};

    // Door to Z4 (at 2619)
    level->walls[level->wallCount++] = (Wall){(Rectangle){2619, 0, 20, 260}, 0.0f};
    level->walls[level->wallCount++] = (Wall){(Rectangle){2619, 380, 20, 500}, 0.0f};
    level->doors[level->doorCount].rect = (Rectangle){2619, 260, 20, 120};
    level->doors[level->doorCount].requiredPerm = PERM_ADMIN;
    level->doors[level->doorCount].isOpen = false;
    level->doors[level->doorCount].animationProgress = 0.0f;
    level->doorCount++;


    // -- Zone 4 (2619..3576, 0..654) -- Executive
    // Top/Bottom
    level->walls[level->wallCount++] = (Wall){(Rectangle){2619, -50, 957, 50}, 0.0f};
    level->walls[level->wallCount++] = (Wall){(Rectangle){2619, 654, 957, 50}, 0.0f};
    level->walls[level->wallCount++] = (Wall){(Rectangle){3576, 0, 50, 654}, 0.0f}; // End

    // Boss Desk / Pillars
    level->walls[level->wallCount++] = (Wall){(Rectangle){3000, 200, 50, 50}, 0.0f};
    level->walls[level->wallCount++] = (Wall){(Rectangle){3000, 400, 50, 50}, 0.0f};
    level->walls[level->wallCount++] = (Wall){(Rectangle){3300, 300, 100, 60}, 0.0f}; // Desk


    // --- ENEMIES ---
    // Zone 1
    level->enemies[level->enemyCount++] = InitEnemy((Vector2){500, 200}, ENEMY_CIVILIAN);
    level->enemies[level->enemyCount++] = InitEnemy((Vector2){500, 450}, ENEMY_CIVILIAN);
    level->enemies[level->enemyCount++] = InitEnemy((Vector2){800, 320}, ENEMY_STAFF); // Key

    // Zone 2
    level->enemies[level->enemyCount++] = InitEnemy((Vector2){1200, 300}, ENEMY_STAFF);
    level->enemies[level->enemyCount++] = InitEnemy((Vector2){1400, 500}, ENEMY_STAFF);
    level->enemies[level->enemyCount++] = InitEnemy((Vector2){1600, 200}, ENEMY_GUARD); // Key

    // Zone 3
    level->enemies[level->enemyCount++] = InitEnemy((Vector2){2000, 100}, ENEMY_GUARD);
    level->enemies[level->enemyCount++] = InitEnemy((Vector2){2300, 700}, ENEMY_GUARD);
    level->enemies[level->enemyCount++] = InitEnemy((Vector2){2500, 320}, ENEMY_ADMIN); // Key

    // Zone 4
    level->enemies[level->enemyCount++] = InitEnemy((Vector2){2800, 200}, ENEMY_ADMIN);
    level->enemies[level->enemyCount++] = InitEnemy((Vector2){2800, 500}, ENEMY_ADMIN);
    level->enemies[level->enemyCount++] = InitEnemy((Vector2){3400, 320}, ENEMY_ADMIN); // Boss
}

void UnloadEpisode2() {
    UnloadTexture(texZone1);
    UnloadTexture(texZone2);
    UnloadTexture(texZone3);
    UnloadTexture(texZone4);
}
