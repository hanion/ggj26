#include "../enemies/enemy.h"
#include "../types.h"
#include "episodes.h"

static Texture2D texZone1;
static Texture2D texZone2;
static Texture2D texZone3;
static Texture2D texZone4;
static Texture2D texZone5;
static Texture2D texZone6;
static Texture2D texZone7;


// Define constants locally if needed, or share via header.
// For now, hardcoded values from main.c are fine or we can redefine macros.
#define ENEMY_SHOOT_INTERVAL 2.0f

// Snake Layout: 7 Zones
  // Z1(0,0)->Z2(1.5k,0)->Z3(3k,0) -> Down -> Z4(3k,1k)->Z5(1.5k,1k)->Z6(0,1k) -> Down -> Z7(0,2k)
  
void InitEpisode1(Level *level) {
  // Identities - Unique Keys per Zone
  Identity idCivilian = {.permissionLevel = PERM_NONE, .color = BLUE, .speed = 220.0f};
  
  // Z1 Key: Staff (Green) - Opens D1
  Identity idKeyZ1 = {.permissionLevel = PERM_STAFF, .color = GREEN, .speed = 210.0f};
  
  // Z2 Key: Guard (Red) - Opens D2
  Identity idKeyZ2 = {.permissionLevel = PERM_GUARD, .color = RED, .speed = 200.0f};

  // Z3 Key: Admin (Purple) - Opens D3
  Identity idKeyZ3 = {.permissionLevel = PERM_ADMIN, .color = PURPLE, .speed = 250.0f};

  // Z4 Key: Guard (Red) - Opens D4 (reusing type but unique var if needed, or just reuse)
  Identity idKeyZ4 = {.permissionLevel = PERM_GUARD, .color = RED, .speed = 200.0f};

  // Z5 Key: Staff (Green) - Opens D5
  Identity idKeyZ5 = {.permissionLevel = PERM_STAFF, .color = GREEN, .speed = 210.0f};

  // Z6 Key: Admin (Purple) - Opens D6
  Identity idKeyZ6 = {.permissionLevel = PERM_ADMIN, .color = PURPLE, .speed = 250.0f};

  // 7 Zones (Exact Texture sizes)
  // Z1: (0, 0)         [913x642]
  // Z2: (913, 0)       [911x661]
  // Z3: (1824, 0)      [957x654]
  // Z4: (1824, 654)    [869x645] (Under Z3 left)
  // Z5: (957, 654)     [867x649] (Left of Z4)
  // Z6: (162, 654)     [795x853] (Left of Z5)
  // Z7: (162, 1507)    [795x805] (Below Z6)

  level->id = 1;
  level->playerSpawn = (Vector2){100.0f, 320.0f}; // Start in Z1
  level->playerStartId = idCivilian;
  level->winX = -1.0f; // Handled by Y check

  level->wallCount = 0;

  // --- External Boundaries ---
// Replace all rectangle assignments with Wall struct assignments
// Example: level->walls[level->wallCount++] = (Wall){ (Rectangle){...}, 0.0f };
// Since there are many, I will use a regex if possible or just replace the assignments.
// Actually, I'll use a smarter regex in my head: `(Rectangle){` -> `(Wall){(Rectangle){` + `}, 0.0f}`
// But multi_replace doesn't support regex in replacement efficiently across many lines without explicit chunks.
// I'll try to replace groups.

// Group 1: Boundaries
  level->walls[level->wallCount++] = (Wall){(Rectangle){-50, -50, 3000, 50}, 0.0f};
  
  // Left Side
  level->walls[level->wallCount++] = (Wall){(Rectangle){-50, 0, 50, 642}, 0.0f}; 
  level->walls[level->wallCount++] = (Wall){(Rectangle){112, 654, 50, 853}, 0.0f};
  level->walls[level->wallCount++] = (Wall){(Rectangle){112, 1507, 50, 805}, 0.0f};

  // Right Side
  level->walls[level->wallCount++] = (Wall){(Rectangle){2781, 0, 50, 654}, 0.0f};
  level->walls[level->wallCount++] = (Wall){(Rectangle){2693, 654, 50, 645}, 0.0f};

  // Bottoms
  level->walls[level->wallCount++] = (Wall){(Rectangle){1824, 1299, 900, 50}, 0.0f};
  level->walls[level->wallCount++] = (Wall){(Rectangle){957, 1303, 900, 50}, 0.0f};
  level->walls[level->wallCount++] = (Wall){(Rectangle){112, 2312, 900, 50}, 0.0f};

  // Right Side of Z7
  level->walls[level->wallCount++] = (Wall){(Rectangle){957, 1507, 20, 805}, 0.0f}; 

  // --- Internal Steps/Gaps ---
  level->walls[level->wallCount++] = (Wall){(Rectangle){0, 642, 913, 20}, 0.0f}; 

  level->walls[level->wallCount++] = (Wall){(Rectangle){913, 661, 911, 20}, 0.0f}; 

  level->walls[level->wallCount++] = (Wall){(Rectangle){1824, 654, 300, 20}, 0.0f}; 
  level->walls[level->wallCount++] = (Wall){(Rectangle){2244, 654, 537, 20}, 0.0f}; 

  level->walls[level->wallCount++] = (Wall){(Rectangle){162, 642, 795, 20}, 0.0f}; 

  // --- Vertical Dividers (Doors) ---
  level->walls[level->wallCount++] = (Wall){(Rectangle){913, 0, 20, 260}, 0.0f};
  level->walls[level->wallCount++] = (Wall){(Rectangle){913, 380, 20, 300}, 0.0f}; 

  level->walls[level->wallCount++] = (Wall){(Rectangle){1824, 0, 20, 260}, 0.0f};
  level->walls[level->wallCount++] = (Wall){(Rectangle){1824, 380, 20, 300}, 0.0f};

  level->walls[level->wallCount++] = (Wall){(Rectangle){1824, 654, 20, 260}, 0.0f};
  level->walls[level->wallCount++] = (Wall){(Rectangle){1824, 1034, 20, 300}, 0.0f};

  level->walls[level->wallCount++] = (Wall){(Rectangle){957, 654, 20, 260}, 0.0f};
  level->walls[level->wallCount++] = (Wall){(Rectangle){957, 1034, 20, 300}, 0.0f};
  level->walls[level->wallCount++] = (Wall){(Rectangle){957, 1334, 20, 173}, 0.0f};

  level->walls[level->wallCount++] = (Wall){(Rectangle){162, 1507, 300, 20}, 0.0f}; 
  level->walls[level->wallCount++] = (Wall){(Rectangle){582, 1507, 400, 20}, 0.0f}; 

  // --- Doors ---
  level->doorCount = 6;
  // D1 (X=913)
  level->doors[0].rect = (Rectangle){913, 260, 20, 120};
  level->doors[0].requiredPerm = PERM_STAFF;

  // D2 (X=1824)
  level->doors[1].rect = (Rectangle){1824, 260, 20, 120};
  level->doors[1].requiredPerm = PERM_GUARD;

  // D3 (Z3->Z4, Y=654)
  level->doors[2].rect = (Rectangle){2124, 654, 120, 20};
  level->doors[2].requiredPerm = PERM_ADMIN;

  // D4 (Z4->Z5, X=1824)
  level->doors[3].rect = (Rectangle){1824, 914, 20, 120};
  level->doors[3].requiredPerm = PERM_GUARD;

  // D5 (Z5->Z6, X=957)
  level->doors[4].rect = (Rectangle){957, 914, 20, 120};
  level->doors[4].requiredPerm = PERM_STAFF;

  // D6 (Z6->Z7, Y=1507)
  level->doors[5].rect = (Rectangle){462, 1507, 120, 20};
  level->doors[5].requiredPerm = PERM_ADMIN;
  
  for(int i=0; i<6; i++) {
      level->doors[i].isOpen = false;
      level->doors[i].animationProgress = 0.0f;
  }

  // --- Enemies (Unique Keys) ---
  level->enemyCount = 7;
  
  // Z1 (450, 320) - Key Z1 (Staff) -> Walker
  level->enemies[0] = InitEnemy((Vector2){450, 320}, ENEMY_STAFF); // Base properties
  level->enemies[0].identity = idKeyZ1; // Override identity if needed
  level->enemies[0].haveMask = true;    // Has Mask
  
  // Z2 (1350, 320) - Key Z2 (Guard) -> Guardian
  level->enemies[1] = InitEnemy((Vector2){1350, 320}, ENEMY_GUARD);
  level->enemies[1].identity = idKeyZ2;
  level->enemies[1].haveMask = false;   // No Mask
  
  // Z3 (2250, 320) - Key Z3 (Admin) -> Guardian
  level->enemies[2] = InitEnemy((Vector2){2250, 320}, ENEMY_ADMIN);
  level->enemies[2].identity = idKeyZ3;
  
  // Z4 (2250, 970) - Key Z4 (Guard) -> Guardian
  level->enemies[3] = InitEnemy((Vector2){2250, 970}, ENEMY_GUARD);
  level->enemies[3].identity = idKeyZ4;
  
  // Z5 (1350, 970) - Key Z5 (Staff) -> Walker
  level->enemies[4] = InitEnemy((Vector2){1350, 970}, ENEMY_STAFF);
  level->enemies[4].identity = idKeyZ5;
  
  // Z6 (500, 970) - Key Z6 (Admin) -> Guardian
  level->enemies[5] = InitEnemy((Vector2){500, 970}, ENEMY_ADMIN);
  level->enemies[5].identity = idKeyZ6;
  
  // Z7 (500, 1900) - Final Guard -> Guardian
  level->enemies[6] = InitEnemy((Vector2){500, 1900}, ENEMY_ADMIN);
  level->enemies[6].identity = idKeyZ3;



    // Load Textures
    if (texZone1.id == 0)texZone1 = LoadTexture("assets/environment/background_1.png");
    if (texZone2.id == 0)texZone2 = LoadTexture("assets/environment/background_2.png");
    if (texZone3.id == 0)texZone3 = LoadTexture("assets/environment/background_3.png");
    if (texZone4.id == 0)texZone4 = LoadTexture("assets/environment/background_4.png");
    if (texZone5.id == 0)texZone5 = LoadTexture("assets/environment/background_5.png");
    if (texZone6.id == 0)texZone6 = LoadTexture("assets/environment/background_6.png");
    if (texZone7.id == 0)texZone7 = LoadTexture("assets/environment/background_7.png");

    level->bgs[0] = (Background){texZone1, (Rectangle){0,0,texZone1.width,texZone1.height}, (Rectangle){0,0,913,642}};
    level->bgs[2] = (Background){texZone2, (Rectangle){0,0,texZone2.width,texZone2.height}, (Rectangle){913,0,911,661}};
    level->bgs[3] = (Background){texZone3, (Rectangle){0,0,texZone3.width,texZone3.height}, (Rectangle){1824,0,957,654}};
    level->bgs[4] = (Background){texZone4, (Rectangle){0,0,texZone4.width,texZone4.height}, (Rectangle){1824,654,869,645}};
    level->bgs[5] = (Background){texZone5, (Rectangle){0,0,texZone5.width,texZone5.height}, (Rectangle){957,654,867,649}};
    level->bgs[6] = (Background){texZone6, (Rectangle){0,0,texZone6.width,texZone6.height}, (Rectangle){162,654,795,853}};
    level->bgs[7] = (Background){texZone7, (Rectangle){0,0,texZone7.width,texZone7.height}, (Rectangle){162,1507,795,805}};
	level->bgs_count = 8;
}

void UnloadEpisode1() {
    UnloadTexture(texZone1);
    UnloadTexture(texZone2);
    UnloadTexture(texZone3);
    UnloadTexture(texZone4);
    UnloadTexture(texZone5);
    UnloadTexture(texZone6);
    UnloadTexture(texZone7);
}
