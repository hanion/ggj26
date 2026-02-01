#include "enemy.h"

// Hardcoded for now, or could be passed/defined elsewhere
#define ENEMY_SHOOT_INTERVAL 2.0f

Entity InitEnemy(Vector2 position, EnemyType type) {
    Entity enemy = {0};
    enemy.type = type;
    enemy.position = position;
    enemy.active = true;
    enemy.isPlayer = false;
    enemy.radius = 20.0f;
    enemy.shootTimer = ENEMY_SHOOT_INTERVAL;
    enemy.health = 100.0f;
    enemy.health = 100.0f;
    enemy.maxHealth = 100.0f;
    enemy.IGotHitImSearchingThePlayerForHowManySeconds = 5.0f; // Default duration

    switch (type) {
        case ENEMY_CIVILIAN:
        case ENEMY_STAFF:
             enemy.aiType = AI_WALKER;
             enemy.sightRange = 1400.0f;
             enemy.sightAngle = 120.0f;
             break;
        case ENEMY_GUARD:
        case ENEMY_ADMIN:
             enemy.aiType = AI_GUARDIAN;
             enemy.sightRange =1600.0f;
             enemy.sightAngle = 120.0f;
             break;
    }
    
    enemy.state = STATE_IDLE;
    enemy.patrolStart = position;
    enemy.active = true;
    
    // Randomize initial rotation for variety
    enemy.rotation = (float)GetRandomValue(0, 360);
    
    enemy.identity = GetIdentity(type);
    return enemy;
}

Identity GetIdentity(EnemyType type) {
    switch (type) {
        case ENEMY_CIVILIAN:
            return (Identity){
                .permissionLevel = PERM_NONE,
                .color = BLUE,
                .speed = 220.0f
            };
        case ENEMY_STAFF:
            return (Identity){
                .permissionLevel = PERM_STAFF,
                .color = GREEN,
                .speed = 210.0f
            };
        case ENEMY_GUARD:
            return (Identity){
                .permissionLevel = PERM_GUARD,
                .color = RED,
                .speed = 200.0f
            };
        case ENEMY_ADMIN:
            return (Identity){
                .permissionLevel = PERM_ADMIN,
                .color = PURPLE,
                .speed = 250.0f
            };
        default:
            return (Identity){0};
    }
}

