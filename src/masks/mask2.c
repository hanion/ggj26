#include "mask.h"
#include <stddef.h>

void Mask2_Apply(Entity *player) {
    if (!player) return;
    // Stealth Mask (e.g. Ghost?)
    player->isInvisible = true; 
}

void Mask2_Remove(Entity *player) {
    if (!player) return;
    player->isInvisible = false;
}
