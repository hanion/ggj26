#include "mask.h"
#include "../../raylib/src/raylib.h"

// Internal prototypes
void Mask1_Apply(Entity *player);
void Mask1_Remove(Entity *player);
void Mask2_Apply(Entity *player);
void Mask2_Remove(Entity *player);

void Masks_Init(void) {
    // Nothing global yet
}

void Masks_Update(Entity *player, float dt) {
    if (!player) return;
    
    // Check active mask
    int activeIdx = -1;
    for(int i=0; i<3; i++) {
        if (player->inventory.maskSlots[i].isActive) {
            activeIdx = i;
            break;
        }
    }

    // Reset defaults first (simplify logic: reset then apply)
    // But we don't want to reset 'speed' to 1.0 if base speed changed?
    // Identity.speed is base. Entity also has speed? 
    // Usually Entity uses identity.speed * multiplier.
    // Let's assume we reset temp flags.
    player->speedMultiplier = 1.0f;
    player->isInvisible = false;

    if (activeIdx != -1) {
        // Decrement Timer
        player->inventory.maskSlots[activeIdx].currentTimer -= dt;
        // TraceLog(LOG_INFO, "Mask Active: Idx %d, Timer %.2f, dt %.4f", activeIdx, player->inventory.maskSlots[activeIdx].currentTimer, dt);

        MaskAbilityType type = player->inventory.maskSlots[activeIdx].type;
        switch(type) {
            case MASK_SPEED: Mask1_Apply(player); break;
            case MASK_STEALTH: Mask2_Apply(player); break;
            default: break;
        }

        // Check Expiration
        if (player->inventory.maskSlots[activeIdx].currentTimer <= 0.0f) {
            TraceLog(LOG_INFO, "Mask Expired! Removing mask at slot %d", activeIdx);
            player->inventory.maskSlots[activeIdx].isActive = false;
            player->inventory.maskSlots[activeIdx].type = MASK_NONE; // EMPTY LOOK
            player->inventory.maskSlots[activeIdx].currentTimer = 0.0f;
            player->inventory.maskSlots[activeIdx].collected = false; // Key for UI reset!
            Masks_Deactivate(player); // Reset stats immediately
        }
    }
}

void Masks_Deactivate(Entity *player) {
    if (!player) return;
    player->speedMultiplier = 1.0f;
    player->isInvisible = false;
}
