#include "mask.h"
#include <stddef.h>

void Mask1_Apply(Entity *player) {
    if (!player) return;
    // Speed Mask (e.g. Rabbit?)
    player->speedMultiplier = 1.5f; 
}

void Mask1_Remove(Entity *player) {
    if (!player) return;
    player->speedMultiplier = 1.0f;
}
