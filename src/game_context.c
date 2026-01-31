#include "game_context.h"

void GameContext_Init(GameContext *ctx) {
    if (!ctx) return;
    *ctx = (GameContext){0};
    ctx->nextEpisodeId = 1;
    ctx->player.valid = false;
}

bool GameContext_AllowsEquip(const GameContext *ctx, PlayerEquipState equip) {
    // Always allowed
    if (equip == PLAYER_EQUIP_BARE_HANDS || equip == PLAYER_EQUIP_KNIFE) return true;

    if (!ctx || !ctx->player.valid) return false;

    switch (equip) {
        case PLAYER_EQUIP_FLASHLIGHT: return ctx->player.hasFlashlight;
        case PLAYER_EQUIP_HANDGUN: return ctx->player.hasHandgun;
        case PLAYER_EQUIP_RIFLE: return ctx->player.hasRifle;
        case PLAYER_EQUIP_SHOTGUN: return ctx->player.hasShotgun;
        default: return false;
    }
}

void GameContext_SaveFromPlayer(GameContext *ctx, const Entity *livePlayer) {
    if (!ctx || !livePlayer) return;

    ctx->player.valid = true;
    ctx->player.identity = livePlayer->identity;
    ctx->player.equipped = livePlayer->equipmentState;
    ctx->player.magAmmo = livePlayer->magAmmo;
    ctx->player.reserveAmmo = livePlayer->reserveAmmo;

    // Inventory inference: if you've ever equipped it, you own it.
    // (Later, real pickups can set these flags directly.)
    switch (livePlayer->equipmentState) {
        case PLAYER_EQUIP_FLASHLIGHT: ctx->player.hasFlashlight = true; break;
        case PLAYER_EQUIP_HANDGUN: ctx->player.hasHandgun = true; break;
        case PLAYER_EQUIP_RIFLE: ctx->player.hasRifle = true; break;
        case PLAYER_EQUIP_SHOTGUN: ctx->player.hasShotgun = true; break;
        default: break;
    }
}
