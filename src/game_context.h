#ifndef GAME_CONTEXT_H
#define GAME_CONTEXT_H

#include <stdbool.h>

#include "entity.h"
#include "types.h"

// This holds progression state shared across screens and later used for saving/loading.
// Keep it independent from rendering/gameplay modules.

typedef struct {
    bool valid;
    Identity identity;
    Inventory inventory;
} PlayerContext;

typedef struct {
    int nextEpisodeId;       // The episode the player should play next.
    bool hasProgress;        // True once the player has started at least one episode.
    bool hasWonLastEpisode;  // Set when the active episode is completed.

    PlayerContext player;
} GameContext;

// Initialize a new (fresh) game context.
void GameContext_Init(GameContext *ctx);

// Whether the player's saved inventory allows equipping the given item.
bool GameContext_AllowsEquip(const GameContext *ctx, PlayerEquipState equip);

// Save carry-over data from the live player object (called on win / checkpoint).
void GameContext_SaveFromPlayer(GameContext *ctx, const Entity *livePlayer);

#endif // GAME_CONTEXT_H
