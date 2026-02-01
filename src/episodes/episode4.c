#include "../enemies/enemy.h"
#include "../levels.h"
#include "../types.h"
#include "episodes.h"

static Texture2D texBackground;

void InitEpisode4(Level *level) {
    level->id = 4;
    level->playerSpawn = (Vector2){200.0f, 320.0f};
    level->playerStartId = GetIdentity(ENEMY_CIVILIAN);

    level->wallCount = 0;
    level->doorCount = 0;
    level->enemyCount = 0;
    level->bgs_count = 0;

    // ---- LEVEL EDITOR EXPORT ----
// ---- WALLS ----
level->wallCount = 18;
level->walls[0] = (Wall){(Rectangle){13, 1, 2597, 113}, 0.00f};
level->walls[1] = (Wall){(Rectangle){30, 112, 113, 2473}, 0.00f};
level->walls[2] = (Wall){(Rectangle){2522, 116, 105, 2465}, 0.00f};
level->walls[3] = (Wall){(Rectangle){128, 2528, 2497, 89}, 0.00f};
level->walls[4] = (Wall){(Rectangle){913, 92, 857, 281}, 0.00f};
level->walls[5] = (Wall){(Rectangle){902, 617, 313, 585}, 0.00f};
level->walls[6] = (Wall){(Rectangle){132, 877, 297, 69}, 0.00f};
level->walls[7] = (Wall){(Rectangle){617, 881, 293, 57}, 0.00f};
level->walls[8] = (Wall){(Rectangle){1456, 612, 313, 589}, 0.00f};
level->walls[9] = (Wall){(Rectangle){1766, 881, 265, 321}, 0.00f};
level->walls[10] = (Wall){(Rectangle){1456, 1432, 581, 341}, 0.00f};
level->walls[11] = (Wall){(Rectangle){910, 1430, 305, 345}, 0.00f};
level->walls[12] = (Wall){(Rectangle){127, 1702, 301, 73}, 0.00f};
level->walls[13] = (Wall){(Rectangle){622, 1704, 309, 65}, 0.00f};
level->walls[14] = (Wall){(Rectangle){915, 2055, 69, 481}, 0.00f};
level->walls[15] = (Wall){(Rectangle){2261, 870, 281, 77}, 0.00f};
level->walls[16] = (Wall){(Rectangle){1767, 2338, 797, 233}, 0.00f};
level->walls[17] = (Wall){(Rectangle){2018, 2098, 681, 153}, -45.00f};

// ---- BACKGROUNDS ----
level->bgs_count = 1;
static Texture2D level_bg_textures[1];
if (0 == level_bg_textures[0].id) level_bg_textures[0] = LoadTexture("assets/environment/back_full2.png");
level->bgs[0] = (Background){level_bg_textures[0], (Rectangle){0, 0, 4096, 4096}, (Rectangle){29, -2, 2612, 2624}};

// ---- DOORS ----
level->doorCount = 9;
level->doors[0].rect = (Rectangle){1737.543091,376.712402,33.000000,229.000000};
level->doors[0].requiredPerm = PERM_STAFF;  // Door 0: GREEN
level->doors[1].rect = (Rectangle){2032.032471,880.386841,229.000000,61.000000};
level->doors[1].requiredPerm = PERM_ADMIN;  // Door 1: PURPLE
level->doors[2].rect = (Rectangle){427.137695,881.744751,189.000000,61.000000};
level->doors[2].requiredPerm = PERM_STAFF;  // Door 2: GREEN
level->doors[3].rect = (Rectangle){432.569794,1708.397095,185.000000,61.000000};
level->doors[3].requiredPerm = PERM_GUARD;  // Door 3: RED
level->doors[4].rect = (Rectangle){902.989258,1204.005493,41.000000,221.000000};
level->doors[4].requiredPerm = PERM_GUARD;  // Door 4: RED
level->doors[5].rect = (Rectangle){1975.231079,1204.366821,65.000000,225.000000};
level->doors[5].requiredPerm = PERM_GUARD;  // Door 5: RED
level->doors[6].rect = (Rectangle){912.832764,1785.312134,69.000000,265.000000};
level->doors[6].requiredPerm = PERM_ADMIN;  // Door 6: PURPLE
level->doors[7].rect = (Rectangle){1220.745605,1705.173584,225.000000,69.000000};
level->doors[7].requiredPerm = PERM_GUARD;  // Door 7: RED
level->doors[8].rect = (Rectangle){1224.236084,617.829712,225.000000,45.000000};
level->doors[8].requiredPerm = PERM_STAFF;  // Door 8: GREEN

// ---- ENEMIES ----
// Identity definitions (like Episode 1)
Identity idCivilian = {.permissionLevel = PERM_STAFF, .color = GREEN, .speed = 220.0f};
Identity idStaff = {.permissionLevel = PERM_GUARD, .color = BLUE, .speed = 210.0f};
Identity idGuard = {.permissionLevel = PERM_GUARD, .color = RED, .speed = 200.0f};
Identity idAdmin = {.permissionLevel = PERM_ADMIN, .color = PURPLE, .speed = 250.0f};

level->enemyCount = 18;

// CIVILIANS (GREEN, PERM_STAFF)
level->enemies[0] = InitEnemy((Vector2){735.745483,735.518005}, ENEMY_CIVILIAN);
level->enemies[0].identity = idCivilian;
level->enemies[0].haveMask = true;

level->enemies[1] = InitEnemy((Vector2){1615.132568,449.892944}, ENEMY_CIVILIAN);
level->enemies[1].identity = idCivilian;
level->enemies[1].haveMask = false;

// GUARDS (RED, PERM_GUARD)
level->enemies[2] = InitEnemy((Vector2){1333.033447,1338.088745}, ENEMY_GUARD);
level->enemies[2].identity = idGuard;
level->enemies[2].haveMask = true;

level->enemies[10] = InitEnemy((Vector2){558.434204,1351.944946}, ENEMY_GUARD);
level->enemies[10].identity = idGuard;
level->enemies[10].haveMask = false;

level->enemies[11] = InitEnemy((Vector2){324.094482,1130.706665}, ENEMY_GUARD);
level->enemies[11].identity = idGuard;
level->enemies[11].haveMask = true;

level->enemies[12] = InitEnemy((Vector2){343.988403,1493.902466}, ENEMY_GUARD);
level->enemies[12].identity = idGuard;
level->enemies[12].haveMask = false;

level->enemies[13] = InitEnemy((Vector2){1613.163086,1340.200439}, ENEMY_GUARD);
level->enemies[13].identity = idGuard;
level->enemies[13].haveMask = true;

level->enemies[14] = InitEnemy((Vector2){1351.677246,1104.445679}, ENEMY_GUARD);
level->enemies[14].identity = idGuard;
level->enemies[14].haveMask = false;

// ADMINS (PURPLE, PERM_ADMIN)
level->enemies[3] = InitEnemy((Vector2){259.235962,1985.612427}, ENEMY_ADMIN);
level->enemies[3].identity = idAdmin;
level->enemies[3].haveMask = true;

level->enemies[4] = InitEnemy((Vector2){809.354004,2123.265869}, ENEMY_ADMIN);
level->enemies[4].identity = idAdmin;
level->enemies[4].haveMask = true;

level->enemies[15] = InitEnemy((Vector2){2004.342529,363.679047}, ENEMY_ADMIN);
level->enemies[15].identity = idAdmin;
level->enemies[15].haveMask = false;

level->enemies[16] = InitEnemy((Vector2){2286.748047,378.160187}, ENEMY_ADMIN);
level->enemies[16].identity = idAdmin;
level->enemies[16].haveMask = false;

level->enemies[17] = InitEnemy((Vector2){2169.802490,619.787598}, ENEMY_ADMIN);
level->enemies[17].identity = idAdmin;
level->enemies[17].haveMask = true;

// STAFF (BLUE, PERM_GUARD)
level->enemies[5] = InitEnemy((Vector2){1212.596924,2263.489990}, ENEMY_STAFF);
level->enemies[5].identity = idStaff;
level->enemies[5].haveMask = false;

level->enemies[6] = InitEnemy((Vector2){1688.469727,2135.659668}, ENEMY_STAFF);
level->enemies[6].identity = idStaff;
level->enemies[6].haveMask = true;

level->enemies[7] = InitEnemy((Vector2){2142.373047,1854.279907}, ENEMY_STAFF);
level->enemies[7].identity = idStaff;
level->enemies[7].haveMask = false;

level->enemies[8] = InitEnemy((Vector2){2271.264648,1554.704346}, ENEMY_STAFF);
level->enemies[8].identity = idStaff;
level->enemies[8].haveMask = false;

level->enemies[9] = InitEnemy((Vector2){2322.656250,1276.331665}, ENEMY_STAFF);
level->enemies[9].identity = idStaff;
level->enemies[9].haveMask = true;

// ---- WIN AREA ----
level->win_area = (Rectangle){287.192993,2234.508301,438.000000,274.000000};
// ---- LEVEL EDITOR EXPORT END ----
}

void UnloadEpisode4() {
    if (texBackground.id != 0) {
        UnloadTexture(texBackground);
        texBackground = (Texture2D){0};
    }
}
