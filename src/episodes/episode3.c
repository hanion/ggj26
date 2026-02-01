#include "../enemies/enemy.h"
#include "../types.h"
#include "episodes.h"
#include <stdio.h> // For getting NULL

void InitEpisode3(Level *level) {
    level->id = 3; // Episode 3
    level->playerSpawn = (Vector2){200.0f, 500.0f};
    level->playerStartId = GetIdentity(ENEMY_CIVILIAN); // Start undercover
    

// ---- LEVEL EDITOR EXPORT ----
// ---- WALLS ----
level->wallCount = 34;
level->walls[0] = (Wall){(Rectangle){-261, 455, 57, 1417}, 0.00f};
level->walls[1] = (Wall){(Rectangle){-218, 1814, 1389, 53}, 0.00f};
level->walls[2] = (Wall){(Rectangle){1118, 426, 57, 1401}, 0.00f};
level->walls[3] = (Wall){(Rectangle){-245, 423, 1385, 57}, 0.00f};
level->walls[4] = (Wall){(Rectangle){416, 469, 89, 285}, 0.00f};
level->walls[5] = (Wall){(Rectangle){401, 826, 125, 85}, 0.00f};
level->walls[6] = (Wall){(Rectangle){-210, 1107, 281, 89}, 0.00f};
level->walls[7] = (Wall){(Rectangle){593, 855, 173, 65}, 0.00f};
level->walls[8] = (Wall){(Rectangle){706, 916, 61, 109}, 0.00f};
level->walls[9] = (Wall){(Rectangle){701, 1104, 81, 333}, 0.00f};
level->walls[10] = (Wall){(Rectangle){153, 859, 177, 61}, 0.00f};
level->walls[11] = (Wall){(Rectangle){154, 918, 65, 273}, 0.00f};
level->walls[12] = (Wall){(Rectangle){153, 1266, 65, 169}, 0.00f};
level->walls[13] = (Wall){(Rectangle){215, 1375, 117, 61}, 0.00f};
level->walls[14] = (Wall){(Rectangle){406, 1377, 117, 93}, 0.00f};
level->walls[15] = (Wall){(Rectangle){597, 1375, 105, 61}, 0.00f};
level->walls[16] = (Wall){(Rectangle){857, 1103, 273, 97}, 0.00f};
level->walls[17] = (Wall){(Rectangle){408, 1536, 97, 285}, 0.00f};
level->walls[18] = (Wall){(Rectangle){38, 686, 137, 73}, 0.00f};
level->walls[19] = (Wall){(Rectangle){35, 1533, 133, 85}, 0.00f};
level->walls[20] = (Wall){(Rectangle){-97, 1759, 121, 61}, 0.00f};
level->walls[21] = (Wall){(Rectangle){24, 1765, 53, 61}, 0.00f};
level->walls[22] = (Wall){(Rectangle){746, 1536, 149, 77}, 0.00f};
level->walls[23] = (Wall){(Rectangle){905, 1739, 129, 77}, 0.00f};
level->walls[24] = (Wall){(Rectangle){650, 1296, 61, 49}, 0.00f};
level->walls[25] = (Wall){(Rectangle){570, 477, 133, 61}, 0.00f};
level->walls[26] = (Wall){(Rectangle){752, 682, 137, 73}, 0.00f};
level->walls[27] = (Wall){(Rectangle){858, 818, 109, 69}, 0.00f};
level->walls[28] = (Wall){(Rectangle){505, 1604, 69, 101}, 0.00f};
level->walls[29] = (Wall){(Rectangle){900, 477, 125, 61}, 0.00f};
level->walls[30] = (Wall){(Rectangle){406, 1078, 113, 25}, 0.00f};
level->walls[31] = (Wall){(Rectangle){408, 1208, 109, 25}, 0.00f};
level->walls[32] = (Wall){(Rectangle){217, 1329, 117, 49}, 0.00f};
level->walls[33] = (Wall){(Rectangle){-119, 523, 113, 49}, 0.00f};

// ---- BACKGROUNDS ----
level->bgs_count = 1;
static Texture2D level_bg_textures[1];
if (0 == level_bg_textures[0].id) level_bg_textures[0] = LoadTexture("assets/environment/background_3_1.png");
level->bgs[0] = (Background){level_bg_textures[0], (Rectangle){0, 0, 8092, 8092}, (Rectangle){-250, 432, 1424, 1440}};

// ---- DOORS ----
level->doorCount = 9;
level->doors[0].rect = (Rectangle){70.951965,1118.683960,81.000000,37.000000};
level->doors[0].requiredPerm = PERM_NONE;
level->doors[1].rect = (Rectangle){330.952087,861.047058,69.000000,49.000000};
level->doors[1].requiredPerm = PERM_STAFF;
level->doors[2].rect = (Rectangle){525.618713,856.476074,69.000000,53.000000};
level->doors[2].requiredPerm = PERM_GUARD;
level->doors[3].rect = (Rectangle){708.285461,1024.475952,57.000000,81.000000};
level->doors[3].requiredPerm = PERM_ADMIN;
level->doors[4].rect = (Rectangle){333.618591,1385.809326,73.000000,33.000000};
level->doors[4].requiredPerm = PERM_GUARD;
level->doors[5].rect = (Rectangle){522.952087,1385.809326,73.000000,25.000000};
level->doors[5].requiredPerm = PERM_ADMIN;
level->doors[6].rect = (Rectangle){445.618835,1471.142700,25.000000,65.000000};
level->doors[6].requiredPerm = PERM_GUARD;
level->doors[7].rect = (Rectangle){781.618713,1128.476074,77.000000,41.000000};
level->doors[7].requiredPerm = PERM_STAFF;
level->doors[8].rect = (Rectangle){437.205566,752.793701,53.000000,73.000000};
level->doors[8].requiredPerm = PERM_STAFF;

// ---- ENEMIES ----
level->enemyCount = 19;
level->enemies[0] = InitEnemy((Vector2){248.285339,953.350525}, ENEMY_ADMIN);
level->enemies[1] = InitEnemy((Vector2){670.952087,949.350525}, ENEMY_ADMIN);
level->enemies[2] = InitEnemy((Vector2){541.618591,605.350525}, ENEMY_GUARD);
level->enemies[3] = InitEnemy((Vector2){1077.618652,526.683899}, ENEMY_GUARD);
level->enemies[4] = InitEnemy((Vector2){375.007324,1192.886353}, ENEMY_STAFF);
level->enemies[5] = InitEnemy((Vector2){-26.884703,1297.867798}, ENEMY_CIVILIAN);
level->enemies[6] = InitEnemy((Vector2){1074.952148,1240.017212}, ENEMY_ADMIN);
level->enemies[7] = InitEnemy((Vector2){815.826294,1478.551147}, ENEMY_STAFF);
level->enemies[8] = InitEnemy((Vector2){387.273926,1069.938232}, ENEMY_STAFF);
level->enemies[9] = InitEnemy((Vector2){-141.461121,1235.264282}, ENEMY_ADMIN);
level->enemies[10] = InitEnemy((Vector2){120.149231,505.509460}, ENEMY_CIVILIAN);
level->enemies[11] = InitEnemy((Vector2){383.875549,651.023682}, ENEMY_CIVILIAN);
level->enemies[12] = InitEnemy((Vector2){446.250031,1299.077393}, ENEMY_STAFF);
level->enemies[13] = InitEnemy((Vector2){602.103516,1249.442627}, ENEMY_CIVILIAN);
level->enemies[14] = InitEnemy((Vector2){69.501358,1234.389038}, ENEMY_STAFF);
level->enemies[15] = InitEnemy((Vector2){357.215790,1154.686523}, ENEMY_STAFF);
level->enemies[16] = InitEnemy((Vector2){670.838989,1704.527954}, ENEMY_CIVILIAN);
level->enemies[17] = InitEnemy((Vector2){823.986328,1077.843384}, ENEMY_GUARD);
level->enemies[18] = InitEnemy((Vector2){-38.013672,763.843384}, ENEMY_GUARD);

// ---- WIN AREA ----
level->win_area = (Rectangle){428.000000,1104.000000,70.000000,106.000000};
// ---- LEVEL EDITOR EXPORT END ----

}

void UnloadEpisode3() {
    // Unload textures if manageable? 
    // Raylib caches usually, but for now we keep it simple.
}
