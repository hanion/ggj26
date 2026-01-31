#include "player_actions.h"

#include "../../raylib/src/raymath.h"

int PlayerActions_GetClosestEnemyInRange(const Level *level, Vector2 position, float range) {
    if (!level) return -1;

    float closestDist = range;
    int closestIndex = -1;
    for (int i = 0; i < level->enemyCount; i++) {
        if (!level->enemies[i].active) {
            continue;
        }
        float dist = Vector2Distance(position, level->enemies[i].position);
        if (dist <= closestDist) {
            closestDist = dist;
            closestIndex = i;
        }
    }
    return closestIndex;
}

void PlayerActions_HandleEnemyKilled(Level *level,
                                    int enemyIndex,
                                    Entity *player,
                                    Entity *droppedMasks,
                                    int maxMasks,
                                    float droppedMaskRadius,
                                    Entity *droppedCards,
                                    int maxCards,
                                    DroppedGun *droppedGuns,
                                    int maxDroppedGuns) {
    if (!level || !player || !droppedMasks) return;
    if (enemyIndex < 0 || enemyIndex >= level->enemyCount) {
        return;
    }
    if (!level->enemies[enemyIndex].active) {
        return;
    }

    level->enemies[enemyIndex].active = false;

    // Check if enemy has a mask to drop
    if (level->enemies[enemyIndex].haveMask) {
        // Find first inactive mask slot
        for (int i = 0; i < maxMasks; i++) {
            if (!droppedMasks[i].active) {
                droppedMasks[i].identity = level->enemies[enemyIndex].identity;
                droppedMasks[i].position = Vector2Add(level->enemies[enemyIndex].position, (Vector2){-15, -15});
                droppedMasks[i].radius = droppedMaskRadius;
                droppedMasks[i].active = true;
                break;
            }
        }
    }
    
    // Drop Permission Card logic
    // Check if enemy has a permission level
    PermissionLevel enemyPerm = level->enemies[enemyIndex].identity.permissionLevel;
    if (enemyPerm > PERM_NONE && droppedCards != 0) {
        for (int i = 0; i < maxCards; i++) {
             if (!droppedCards[i].active) {
                 droppedCards[i].active = true;
                 droppedCards[i].position = Vector2Add(level->enemies[enemyIndex].position, (Vector2){15, -15});
                 droppedCards[i].radius = 10.0f; // Slightly smaller?
                 droppedCards[i].identity.permissionLevel = enemyPerm;
                 droppedCards[i].identity.color = level->enemies[enemyIndex].identity.color; // Match enemy color
                 break;
             }
        }
    }

    // Drop Gun Logic
    GunType dropType = GUN_NONE;
    PermissionLevel pLevel = level->enemies[enemyIndex].identity.permissionLevel;
    
    if (pLevel == PERM_GUARD) dropType = GUN_HANDGUN;
    else if (pLevel == PERM_ADMIN) dropType = GUN_RIFLE;
    
    if (dropType != GUN_NONE && droppedGuns != 0) {
         for (int i = 0; i < maxDroppedGuns; i++) {
             if (!droppedGuns[i].active) {
                 droppedGuns[i].active = true;
                 droppedGuns[i].position = Vector2Add(level->enemies[enemyIndex].position, (Vector2){0, 15});
                 droppedGuns[i].radius = 15.0f;
                 
                 // Initialize basic gun stats (should strictly come from a factory or define, but hardcoding for drop instance)
                 droppedGuns[i].gun.type = dropType;
                 droppedGuns[i].gun.active = true;
                 // Fill ammo for pickup
                 if (dropType == GUN_HANDGUN) {
                     droppedGuns[i].gun.maxAmmo = 12; // Example
                     droppedGuns[i].gun.currentAmmo = 6; // Randomize?
                     droppedGuns[i].gun.reserveAmmo = 12;
                     droppedGuns[i].gun.reloadTime = 1.5f;
                     droppedGuns[i].gun.cooldown = 0.5f;
                     droppedGuns[i].gun.damage = 30.0f;
                     droppedGuns[i].gun.range = 300.0f;
                 } else if (dropType == GUN_RIFLE) {
                     droppedGuns[i].gun.maxAmmo = 30;
                     droppedGuns[i].gun.currentAmmo = 15;
                     droppedGuns[i].gun.reserveAmmo = 30;
                     droppedGuns[i].gun.reloadTime = 2.0f;
                     droppedGuns[i].gun.cooldown = 0.1f;
                     droppedGuns[i].gun.damage = 35.0f;
                     droppedGuns[i].gun.range = 500.0f;
                 }
                 break;
             }
         }
    }

    // Auto-equip handgun after a kill if we are using knife/bare hands
    GunType currentType = player->inventory.gunSlots[player->inventory.currentGunIndex].type;
    if (currentType == GUN_KNIFE || currentType == GUN_NONE) {
        // Find a handgun or better
        for (int i=0; i<3; i++) { // Using 3 for MAX_GUN_SLOTS which is defined in types.h
             if (player->inventory.gunSlots[i].type == GUN_HANDGUN) {
                  player->inventory.currentGunIndex = i;
                  TraceLog(LOG_INFO, "Player auto-switched to Handgun slot %d", i);
                  break;
             }
        }
    }
}

void PlayerActions_ApplyDamage(Level *level, 
                               int enemyIndex, 
                               float damage,
                               Entity *player,
                               Entity *droppedMasks,
                               int maxMasks,
                               float droppedMaskRadius,
                               Entity *droppedCards,
                               int maxCards,
                               DroppedGun *droppedGuns,
                               int maxDroppedGuns) {
    if (enemyIndex < 0 || enemyIndex >= level->enemyCount) return;
    if (!level->enemies[enemyIndex].active) return;

    level->enemies[enemyIndex].health -= damage;
    
    // Feedback? (Flash white, sound, particles?)
    
    if (level->enemies[enemyIndex].health <= 0) {
        PlayerActions_HandleEnemyKilled(level, enemyIndex, player, droppedMasks, maxMasks, droppedMaskRadius, droppedCards, maxCards, droppedGuns, maxDroppedGuns);
    } else {

        // SURVIVED: Reaction Logic
        Entity *enemy = &level->enemies[enemyIndex];
        
        // 1. Alert the enemy
        enemy->lastKnownPlayerPos = player->position;
        
        // 2. Check Visibility immediately
        // We need to declare CheckLineOfSight prototype or include "enemies/enemy.h"? 
        // "player_actions.c" includes "player_actions.h". "enemy.h" defines Entity.
        // We need "enemy.h" for CheckLineOfSight. It is likely not included here yet?
        // Wait, "player_actions.c" doesn't include "enemy.h" explicitly in the snippet above?
        // Ah, "player_actions.h" probably includes types.
        // I need to check if CheckLineOfSight is available.
        // Just in case, I will assume I need to handle it or blindly set state.
        
        // Actually, Step 1131 showed CheckLineOfSight is in enemy1.c and not exported in header?
        // Step 1079 showed enemy.h has UpdateEnemy.
        // I'll check enemy.h content first to be safe, but assuming I can't call it easily:
        // I'll just set it to SEARCH if I want to be safe, but User said "If cant see player... it will search".
        // Use CheckLineOfSight if available. If not, I'll export it.
        
        // Let's TRY to use CheckLineOfSight. If it fails linking/compling, I'll fix.
        // But better: User said "If cant see player even if in that position".
        // This effectively means: Set to Search/Attack. UpdateEnemy will decide transition?
        // NO, UpdateEnemy transitions *from* Search *to* Attack if visible.
        // But DOES NOT transition Attack -> Search unless lost.
        
        // So: Set to STATE_SEARCH initially.
        // Set searchTimer = IGotHitImSearchingThePlayerForHowManySeconds.
        // IF they see the player next frame (UpdateEnemy), they will switch to ATTACK immediately.
        // IF they don't see the player, they will remain in SEARCH for the duration.
        // This satisfies the condition PERFECTLY without needing CheckLineOfSight here!
        
        enemy->state = STATE_SEARCH;
        enemy->searchTimer = enemy->IGotHitImSearchingThePlayerForHowManySeconds;
    }
}
