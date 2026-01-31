#ifndef MASK_H
#define MASK_H

#include "../entity.h"

// Initialize/Reset masks or mask system if needed
void Masks_Init(void);

// Apply the effect of a specific mask type to the player
// This runs once when mask is activated or every frame if check needed?
// Better: UpdateMask logic runs every frame.
void Masks_Update(Entity *player, float dt);

// Deactivate mask effects
void Masks_Deactivate(Entity *player);

#endif // MASK_H
