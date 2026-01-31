#include "../enemies/enemy.h"
#include "../types.h"
#include "episodes.h"


// Define constants locally if needed, or share via header.
// For now, hardcoded values from main.c are fine or we can redefine macros.
#define ENEMY_SHOOT_INTERVAL 2.0f

// Snake Layout: 7 Zones
  // Z1(0,0)->Z2(1.5k,0)->Z3(3k,0) -> Down -> Z4(3k,1k)->Z5(1.5k,1k)->Z6(0,1k) -> Down -> Z7(0,2k)
  
void InitEpisode1(Level *level) {
  // Identities - Unique Keys per Zone
  Identity idCivilian = {.permissionLevel = PERM_CIVILIAN, .abilities = ABILITY_SHOOT, .color = BLUE, .speed = 220.0f};
  
  // Z1 Key: Staff (Green) - Opens D1
  Identity idKeyZ1 = {.permissionLevel = PERM_STAFF, .abilities = ABILITY_SHOOT, .color = GREEN, .speed = 210.0f};
  
  // Z2 Key: Guard (Red) - Opens D2
  Identity idKeyZ2 = {.permissionLevel = PERM_GUARD, .abilities = ABILITY_SHOOT | ABILITY_PUNCH, .color = RED, .speed = 200.0f};

  // Z3 Key: Admin (Purple) - Opens D3
  Identity idKeyZ3 = {.permissionLevel = PERM_ADMIN, .abilities = ABILITY_SHOOT | ABILITY_PUNCH | ABILITY_DASH, .color = PURPLE, .speed = 250.0f};

  // Z4 Key: Guard (Red) - Opens D4 (reusing type but unique var if needed, or just reuse)
  Identity idKeyZ4 = {.permissionLevel = PERM_GUARD, .abilities = ABILITY_SHOOT, .color = RED, .speed = 200.0f};

  // Z5 Key: Staff (Green) - Opens D5
  Identity idKeyZ5 = {.permissionLevel = PERM_STAFF, .abilities = ABILITY_SHOOT, .color = GREEN, .speed = 210.0f};

  // Z6 Key: Admin (Purple) - Opens D6
  Identity idKeyZ6 = {.permissionLevel = PERM_ADMIN, .abilities = ABILITY_SHOOT | ABILITY_DASH, .color = PURPLE, .speed = 250.0f};

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
  // Top (Z1, Z2, Z3)
  level->walls[level->wallCount++] = (Rectangle){-50, -50, 3000, 50};
  
  // Left Side
  // Z1 Left
  level->walls[level->wallCount++] = (Rectangle){-50, 0, 50, 642}; 
  // Z6 Left (Starts at X=162) -> Need Wall at X=112? No, Z6 is at X=162. 
  // Void to the left of Z6 (0..162).
  // So left wall of Z6 is at 162-50 = 112.
  level->walls[level->wallCount++] = (Rectangle){112, 654, 50, 853};
  // Z7 Left
  level->walls[level->wallCount++] = (Rectangle){112, 1507, 50, 805};

  // Right Side
  // Z3 Right (X=1824+957=2781)
  level->walls[level->wallCount++] = (Rectangle){2781, 0, 50, 654};
  // Z4 Right (X=1824+869=2693)
  level->walls[level->wallCount++] = (Rectangle){2693, 654, 50, 645};

  // Bottoms
  // Z4 Bottom (Y=654+645=1299) -> X=1824..2693
  level->walls[level->wallCount++] = (Rectangle){1824, 1299, 900, 50};
  // Z5 Bottom (Y=654+649=1303) -> X=957..1824
  level->walls[level->wallCount++] = (Rectangle){957, 1303, 900, 50};
  // Z7 Bottom (Y=1507+805=2312) -> X=162..957
  level->walls[level->wallCount++] = (Rectangle){112, 2312, 900, 50};

  // Right Side of Z7
  // Z6 Right (X=162+795=957) is handled by dividers.
  // Z7 Right (X=162+795=957) needs a wall.
  level->walls[level->wallCount++] = (Rectangle){957, 1507, 20, 805}; // Covers Z7 right only

  // --- Internal Steps/Gaps ---
  // Step Z1 bottom / Z6 top gap logic
  // Z1 Bottom is 642. Z6 Top is 654. Gap 12px.
  // Z1 Width 913. Z6 starts at 162.
  // Wall at Y=642 from X=0 to 913 (Bottom of Z1)
  // Connections:
  // Z1 -> Z2 (Horizontal)
  // Z2 -> Z3 (Horizontal)
  // Z3 -> Z4 (Vertical or Corner?) Z3 is (1824,0). Z4 is (1824,654). Distinct rects.
  // Z4 -> Z5 (Horizontal)
  // Z5 -> Z6 (Horizontal)
  // Z6 -> Z7 (Vertical)

  // Block Z1 bottom fully? Yes, Z1 only connects to Z2.
  level->walls[level->wallCount++] = (Rectangle){0, 642, 913, 20}; // Z1 Bottom (Thinner)

  // Block Z2 bottom (Height 661). 
  level->walls[level->wallCount++] = (Rectangle){913, 661, 911, 20}; // Z2 Bottom (Thinner)

  // Block Z3 Bottom? (Height 654)
  // Z3 connects to Z4. Z3 is x=1824..2781. Z4 is x=1824..2693. Y=654.
  // They share the boundary Y=654.
  // Wall at Y=654, check door.
  level->walls[level->wallCount++] = (Rectangle){1824, 654, 300, 20}; // Left part (Thinner)
  level->walls[level->wallCount++] = (Rectangle){2244, 654, 500, 20}; // Right part (Gap 120 at 2124)

  // Block Z6 Top (Y=654) ?? 
  // Z6 is (162, 654). Z1 is above it (0..913, 0..642).
  // Wall at Y=642 covers Z1 bottom.
  // Need wall at Y=654 for Z6 top? Yes, to be safe.
  level->walls[level->wallCount++] = (Rectangle){162, 642, 795, 20}; // Thin filler (Thinner)

  // --- Vertical Dividers (Doors) ---
  // D1: Z1 -> Z2 (X=913)
  level->walls[level->wallCount++] = (Rectangle){913, 0, 20, 260};
  level->walls[level->wallCount++] = (Rectangle){913, 380, 20, 300}; // Gap

  // D2: Z2 -> Z3 (X=1824)
  level->walls[level->wallCount++] = (Rectangle){1824, 0, 20, 260};
  level->walls[level->wallCount++] = (Rectangle){1824, 380, 20, 300};

  // D4: Z4 -> Z5 (X=1824, Y=654..1300)
  level->walls[level->wallCount++] = (Rectangle){1824, 654, 20, 260};
  level->walls[level->wallCount++] = (Rectangle){1824, 1034, 20, 300};

  // D5: Z5 -> Z6 (X=957, Y=654..1300)
  // Z5 Left is 957. Z6 Right is 957.
  // Wall at 957.
  level->walls[level->wallCount++] = (Rectangle){957, 654, 20, 260};
  level->walls[level->wallCount++] = (Rectangle){957, 1034, 20, 300};

  // D6: Z6 -> Z7 (Y=1507).
  // Z6 Bottom is 654+853 = 1507.
  level->walls[level->wallCount++] = (Rectangle){162, 1507, 300, 20}; 
  level->walls[level->wallCount++] = (Rectangle){582, 1507, 400, 20}; // Gap 120

  // --- Doors ---
  level->doorCount = 6;
  // D1 (X=913)
  level->doors[0] = (Rectangle){913, 260, 20, 120};
  level->doorPerms[0] = PERM_STAFF;

  // D2 (X=1824)
  level->doors[1] = (Rectangle){1824, 260, 20, 120};
  level->doorPerms[1] = PERM_GUARD;

  // D3 (Z3->Z4, Y=654)
  level->doors[2] = (Rectangle){2124, 654, 120, 20};
  level->doorPerms[2] = PERM_ADMIN;

  // D4 (Z4->Z5, X=1824)
  level->doors[3] = (Rectangle){1824, 914, 20, 120};
  level->doorPerms[3] = PERM_GUARD;

  // D5 (Z5->Z6, X=957)
  level->doors[4] = (Rectangle){957, 914, 20, 120};
  level->doorPerms[4] = PERM_STAFF;

  // D6 (Z6->Z7, Y=1507)
  level->doors[5] = (Rectangle){462, 1507, 120, 20};
  level->doorPerms[5] = PERM_ADMIN;
  
  for(int i=0; i<6; i++) level->doorsOpen[i] = false;

  // --- Enemies (Unique Keys) ---
  level->enemyCount = 7;
  // Z1 (450, 320) - Key Z1 (Staff)
  level->enemies[0] = (Entity){.position={450, 320}, .active=true, .isPlayer=false, .radius=20, .identity=idKeyZ1, .shootTimer=ENEMY_SHOOT_INTERVAL};
  
  // Z2 (1350, 320) - Key Z2 (Guard)
  level->enemies[1] = (Entity){.position={1350, 320}, .active=true, .isPlayer=false, .radius=20, .identity=idKeyZ2, .shootTimer=ENEMY_SHOOT_INTERVAL};
  
  // Z3 (2250, 320) - Key Z3 (Admin) - **FIXED PROGRESSION**
  level->enemies[2] = (Entity){.position={2250, 320}, .active=true, .isPlayer=false, .radius=20, .identity=idKeyZ3, .shootTimer=ENEMY_SHOOT_INTERVAL};
  
  // Z4 (2250, 970) - Key Z4 (Guard)
  level->enemies[3] = (Entity){.position={2250, 970}, .active=true, .isPlayer=false, .radius=20, .identity=idKeyZ4, .shootTimer=ENEMY_SHOOT_INTERVAL};
  
  // Z5 (1350, 970) - Key Z5 (Staff)
  level->enemies[4] = (Entity){.position={1350, 970}, .active=true, .isPlayer=false, .radius=20, .identity=idKeyZ5, .shootTimer=ENEMY_SHOOT_INTERVAL};
  
  // Z6 (500, 970) - Key Z6 (Admin)
  level->enemies[5] = (Entity){.position={500, 970}, .active=true, .isPlayer=false, .radius=20, .identity=idKeyZ6, .shootTimer=ENEMY_SHOOT_INTERVAL};
  
  // Z7 (500, 1900) - Final Guard
  level->enemies[6] = (Entity){.position={500, 1900}, .active=true, .isPlayer=false, .radius=20, .identity=idKeyZ3, .shootTimer=ENEMY_SHOOT_INTERVAL};
}
