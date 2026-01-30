#include "enemy.h"

// Hardcoded for now, or could be passed/defined elsewhere
#define ENEMY_SHOOT_INTERVAL 2.0f

Entity InitEnemy(Vector2 position, EnemyType type) {
    Entity enemy = {0};
    enemy.position = position;
    enemy.active = true;
    enemy.isPlayer = false;
    enemy.radius = 20.0f;
    enemy.shootTimer = ENEMY_SHOOT_INTERVAL;

    enemy.identity = GetIdentity(type);
    return enemy;
}

Identity GetIdentity(EnemyType type) {
    switch (type) {
        case ENEMY_CIVILIAN:
            return (Identity){
                .permissionLevel = PERM_CIVILIAN,
                .abilities = ABILITY_SHOOT,
                .color = BLUE,
                .speed = 220.0f
            };
        case ENEMY_STAFF:
            return (Identity){
                .permissionLevel = PERM_STAFF,
                .abilities = ABILITY_SHOOT,
                .color = GREEN,
                .speed = 210.0f
            };
        case ENEMY_GUARD:
            return (Identity){
                .permissionLevel = PERM_GUARD,
                .abilities = ABILITY_SHOOT | ABILITY_PUNCH,
                .color = RED,
                .speed = 200.0f
            };
        case ENEMY_ADMIN:
            return (Identity){
                .permissionLevel = PERM_ADMIN,
                .abilities = ABILITY_SHOOT | ABILITY_PUNCH | ABILITY_DASH,
                .color = PURPLE,
                .speed = 250.0f
            };
        default:
            return (Identity){0};
    }
}

