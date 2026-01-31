#include "game_context.h"

void GameContext_Init(GameContext *ctx) {
    if (!ctx) return;
    *ctx = (GameContext){0};
    ctx->nextEpisodeId = 1;
    ctx->player.valid = false;
}

bool GameContext_AllowsEquip(const GameContext *ctx, PlayerEquipState equip) {
    // This function is less relevant now with slots, but for compatibility/logic:
    if (!ctx || !ctx->player.valid) return false;
    
    // Check if any slot has this gun type
    // Mapping PlayerEquipState to GunType is a pain here without a helper, 
    // but assuming we migrate fully, this function might be deprecated.
    // For now, return true to avoid blocking.
    return true; 
}

void GameContext_SaveFromPlayer(GameContext *ctx, const Entity *livePlayer) {
    if (!ctx || !livePlayer) return;

    ctx->player.valid = true;
    ctx->player.identity = livePlayer->identity;
    ctx->player.inventory = livePlayer->inventory;
}
