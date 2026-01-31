#ifndef HUD_H
#define HUD_H

#include "../entity.h"

// Lightweight UI/HUD module.
// - owns any HUD textures (e.g. profile portrait)
// - draws HUD elements in screen-space

void Hud_Init(void);
void Hud_Shutdown(void);

// Draw the in-game HUD related to the player (profile portrait, level, ammo, etc.).
void Hud_DrawPlayer(const Entity *player);

#endif // HUD_H
