#include "../../raylib/src/raylib.h"
#include "../levels.h"
#include "../player/player.h"

#ifndef NPC_H
#define NPC_H

// Draw all NPCs for a given level and allow simple interaction with the player.
void Npc_DrawAll(const Level *level, const Entity *player);
void Npc_UpdateAll(Level *level, float dt, Entity *player);

#endif // NPC_H