#include "npc.h"
#include "levels.h"
#include "gameplay_helpers.h"
#include <stdio.h>

void Npc_DrawAll(const Level *level, const Entity *player) {
    if (!level || !player) return;

    for (int i = 0; i < level->npcCount; i++) {
        const NPC *n = &level->npcs[i];
        if (n->frameCount <= 0) continue; // nothing to draw
        int idx = n->frameIndex % n->frameCount;
        Texture2D tex = n->frames[idx];
        if (!tex.id) {
            TraceLog(LOG_WARNING, "NPC '%s' frame %d texture not loaded", n->name, idx);
            continue;
        }
        float scale = (n->scale > 0.0f) ? n->scale : 1.0f;
        float w = tex.width * scale;
        float h = tex.height * scale;
        Rectangle src = (Rectangle){0, 0, (float)tex.width, (float)tex.height};
        Rectangle dst = (Rectangle){n->position.x - w*0.5f, n->position.y - h*0.5f, w, h};
        DrawTexturePro(tex, src, dst, (Vector2){0,0}, 0, n->tint);

        float dist = Vector2Distance(player->position, n->position);
        if (dist < n->radius + player->radius + 20.0f) {
            DrawText("PRESS SPACE TO TALK", (int)n->position.x - 60, (int)n->position.y - (tex.height/2) - 20, 12, YELLOW);
            if (IsKeyPressed(KEY_SPACE)) {
                DrawText(TextFormat("%s: Merhaba!", n->name), (int)n->position.x - 40, (int)n->position.y - (tex.height/2) - 40, 12, WHITE);
            }
        }
    }
}

void Npc_UpdateAll(Level *level, float dt, Entity *player) {
    if (!level || !player) return;

    // --- NPC collision (treat NPCs as circular obstacles) ---
    // Assumes there is exactly one player entity stored in the level.
    // If your project stores the player elsewhere, move this logic to the main update where the player is available.
    if (player->radius > 0.0f) {
        for (int i = 0; i < level->npcCount; i++) {
            NPC *n = &level->npcs[i];
            float minDist = n->radius + player->radius;
            Vector2 delta = Vector2Subtract(player->position, n->position);
            float dist = Vector2Length(delta);
            if (dist > 0.0f && dist < minDist) {
                Vector2 pushDir = Vector2Scale(delta, 1.0f / dist);
                float push = (minDist - dist);
                player->position = Vector2Add(player->position, Vector2Scale(pushDir, push));
            }
        }
    }

    // --- Animation update ---
    for (int i = 0; i < level->npcCount; i++) {
        NPC *n = &level->npcs[i];
        if (n->frameCount <= 1) continue;
        n->frameTimer += dt;
        if (n->frameDuration > 0.0f && n->frameTimer >= n->frameDuration) {
            n->frameTimer = 0.0f;
            n->frameIndex = (n->frameIndex + 1) % n->frameCount;
        }
    }
}
