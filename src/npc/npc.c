#include "npc.h"
#include "levels.h"
#include "gameplay_helpers.h"
#include <stdio.h>
#include <string.h>
extern bool gameWon;

static void Npc_StartDialogue(Level *level, const NPC *n, const char *line, bool isPlayer) {
    level->activeDialogueText = line;
    level->activeDialoguePos = isPlayer ? level->activeDialoguePos : (Vector2){ n->position.x, n->position.y - 60.0f };
    level->activeDialogueTimer = 2.0f;
    level->activeDialogueIsPlayer = isPlayer;
}

// helper: draw a simple hold bar
static void DrawHoldBar(float x, float y, float w, float h, float t) {
    if (t < 0.0f) t = 0.0f;
    if (t > 1.0f) t = 1.0f;
    DrawRectangle((int)x, (int)y, (int)w, (int)h, (Color){0, 0, 0, 160});
    DrawRectangle((int)x + 2, (int)y + 2, (int)((w - 4) * t), (int)(h - 4), (Color){255, 255, 255, 220});
}

void Npc_DrawAll(const Level *level, const Entity *player) {
    if (!level || !player) return;

    for (int i = 0; i < level->npcCount; i++) {
        const NPC *n = &level->npcs[i];
        if (n->frameCount <= 0) continue;
        int idx = n->frameIndex % n->frameCount;
        Texture2D tex = n->frames[idx];
        if (!tex.id) continue;

        float scale = (n->scale > 0.0f) ? n->scale : 1.0f;
        float w = tex.width * scale;
        float h = tex.height * scale;
        Rectangle src = (Rectangle){0, 0, (float)tex.width, (float)tex.height};
        Rectangle dst = (Rectangle){n->position.x - w*0.5f, n->position.y - h*0.5f, w, h};
        DrawTexturePro(tex, src, dst, (Vector2){0,0}, 0, n->tint);

        float dist = Vector2Distance(player->position, n->position);
        if (dist < n->radius + player->radius + 20.0f) {
            DrawText("PRESS SPACE TO TALK", (int)n->position.x - 60, (int)n->position.y - (int)(h*0.5f) - 20, 12, YELLOW);
        }
    }

    // Dialogue line (one at a time)
    if (level->activeDialogueText && level->activeDialogueTimer > 0.0f) {
        Vector2 pos = level->activeDialoguePos;
        const char *prefix = level->activeDialogueIsPlayer ? "YOU: " : "NPC: ";
        DrawText(prefix, (int)pos.x - 90, (int)pos.y - 16, 12, (Color){200, 200, 200, 255});
        DrawText(level->activeDialogueText, (int)pos.x - 90, (int)pos.y, 14, WHITE);
    }

    // Outro line after all dialogues
    if (level->showOutroLine && level->outroLineTimer > 0.0f) {
        const char *msg = "Git buradan.";
        int fontSize = 42;
        int tw = MeasureText(msg, fontSize);
        DrawText(msg, (GetScreenWidth() - tw) / 2, 80, fontSize, WHITE);
    }

    // Big prompt above sigaraci after dialogue, until mask is taken
    if (level->showTakeMaskPrompt && !level->maskTaken) {
        float distToMask = Vector2Distance(player->position, level->takeMaskPos);
        // bigger, more forgiving range
        float requiredDist = player->radius + 140.0f;
        bool inRange = distToMask <= requiredDist;

        const char *msg = "TAKE THE MASK";
        int fontSize = 42;
        int tw = MeasureText(msg, fontSize);
        int x = (int)level->takeMaskPos.x - tw / 2;
        int y = (int)level->takeMaskPos.y - 140;
        DrawText(msg, x, y, fontSize, WHITE);

        const char *hint = inRange ? "HOLD E" : "GET CLOSER";
        int hw = MeasureText(hint, 18);
        DrawText(hint, (int)level->takeMaskPos.x - hw / 2, y + fontSize + 6, 18, YELLOW);

        // Only show the loading bar when in range
        if (inRange) {
            float t = (level->takeMaskHoldRequired > 0.0f) ? (level->takeMaskHold / level->takeMaskHoldRequired) : 0.0f;
            DrawHoldBar((float)((int)level->takeMaskPos.x - 80), (float)(y + fontSize + 28), 160, 14, t);
        }

        // DEBUG: remove later
        DrawText(TextFormat("maskDist=%.1f req=%.1f inRange=%d E=%d hold=%.2f/%.2f",
                            distToMask,
                            requiredDist,
                            inRange,
                            IsKeyDown(KEY_E),
                            level->takeMaskHold,
                            level->takeMaskHoldRequired),
                 10, 10, 12, GREEN);
    }

    // Success message
    if (level->maskTakenMsgTimer > 0.0f) {
        const char *msg = "I BECOME HIM";
        int fontSize = 42;
        int tw = MeasureText(msg, fontSize);
        int x = (int)level->takeMaskPos.x - tw / 2;
        int y = (int)level->takeMaskPos.y - 140;
        DrawText(msg, x, y, fontSize, WHITE);
    }
}

void Npc_UpdateAll(Level *level, float dt, Entity *player) {
    if (!level || !player) return;

    // Keep player dialogue anchored above player (lower than before)
    if (level->activeDialogueText && level->activeDialogueIsPlayer) {
        level->activeDialoguePos = (Vector2){ player->position.x, player->position.y - 45.0f };
    }

    // Keep takeMaskPos synced to Sigaraci NPC (prevents stale/zero position)
    if (level->showTakeMaskPrompt) {
        for (int i = 0; i < level->npcCount; i++) {
            NPC *n = &level->npcs[i];
            if (n->name && strcmp(n->name, "Sigaraci") == 0) {
                level->takeMaskPos = n->position;
                break;
            }
        }
    }

    // outro timer
    if (level->outroLineTimer > 0.0f) {
        level->outroLineTimer -= dt;
        if (level->outroLineTimer < 0.0f) level->outroLineTimer = 0.0f;
    }

    // dialogue timer
    if (level->activeDialogueTimer > 0.0f) {
        level->activeDialogueTimer -= dt;
        if (level->activeDialogueTimer <= 0.0f) {
            level->activeDialogueTimer = 0.0f;
            level->activeDialogueText = NULL;
        }
    }

    // Success message timer -> triggers level end
    if (level->maskTakenMsgTimer > 0.0f) {
        level->maskTakenMsgTimer -= dt;
        if (level->maskTakenMsgTimer <= 0.0f) {
            level->maskTakenMsgTimer = 0.0f;
            level->showOutroLine = true; // Trigger level transition in prolog.c
            gameWon = 1; // Set game won flag
        }
    }

    // --- NPC collision ---
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

    // --- Dialogue trigger / advance ---
    if (!level->showTakeMaskPrompt && IsKeyPressed(KEY_SPACE)) {
        for (int i = 0; i < level->npcCount; i++) {
            NPC *n = &level->npcs[i];
            float dist = Vector2Distance(player->position, n->position);
            if (dist > n->radius + player->radius + 20.0f) continue;

            bool isSigaraci = (n->name && strcmp(n->name, "Sigaraci") == 0);
            if (isSigaraci) {
                bool othersDone = true;
                for (int j = 0; j < level->npcCount; j++) {
                    if (j == i) continue;
                    if (!level->npcs[j].dialogueCompleted) { othersDone = false; break; }
                }
                if (!othersDone) {
                    Npc_StartDialogue(level, n, "Talk to the others first.", false);
                    break;
                }
            }

            if (!n->dialogueLines || n->dialogueLineCount <= 0) break;

            int turn = n->dialogueLineIndex;
            bool playerTurn = (turn % 2) == 1;
            int pairIndex = turn / 2;

            if (!playerTurn) {
                // NPC speaks
                if (pairIndex >= n->dialogueLineCount) pairIndex = n->dialogueLineCount - 1;
                Npc_StartDialogue(level, n, n->dialogueLines[pairIndex], false);
            } else {
                // Player replies (if defined), else "..."
                const char *reply = "...";
                if (n->playerReplyLines && n->playerReplyLineCount > 0) {
                    int ri = pairIndex;
                    if (ri < 0) ri = 0;
                    if (ri >= n->playerReplyLineCount) ri = n->playerReplyLineCount - 1;
                    reply = n->playerReplyLines[ri];
                }
                // set player dialogue position above player
                level->activeDialoguePos = (Vector2){ player->position.x, player->position.y - 45.0f };
                Npc_StartDialogue(level, n, reply, true);
            }

            n->dialogueLineIndex++;

            // completed when NPC line + player line for each pair is done
            int requiredTurns = n->dialogueLineCount * 2;
            if (n->dialogueLineIndex >= requiredTurns) {
                n->dialogueCompleted = true;

                if (isSigaraci) {
                    // do not auto-trigger mask here; keep existing flow (mask triggers when sigaraci dialogue ends)
                    level->showTakeMaskPrompt = true;
                    level->takeMaskPos = n->position;
                    level->takeMaskHold = 0.0f;
                }

                // check if all NPC dialogues done -> show outro
                bool allDone = true;
                for (int k = 0; k < level->npcCount; k++) {
                    if (!level->npcs[k].dialogueCompleted) { allDone = false; break; }
                }
                if (allDone) {
                    level->showOutroLine = true;
                    level->outroLineTimer = 3.0f;
                }
            }

            break;
        }
    }

    // --- Hold E near sigaraci to take mask ---
    if (level->showTakeMaskPrompt && !level->maskTaken) {
        // Fix: if not initialized, set a sane default
        if (level->takeMaskHoldRequired <= 0.0f) {
            level->takeMaskHoldRequired = 1.25f;
        }

        float dist = Vector2Distance(player->position, level->takeMaskPos);
        float requiredDist = player->radius + 140.0f;
        int inRange = (dist <= requiredDist);
        int eDown = IsKeyDown(KEY_E);

        // DEBUG: throttle logs
        static float dbgTimer = 0.0f;
        dbgTimer += dt;
        if (dbgTimer >= 0.5f) {
            dbgTimer = 0.0f;
            TraceLog(LOG_INFO,
                     "[MASK] dt=%.3f dist=%.1f req=%.1f inRange=%d E=%d hold=%.2f/%.2f",
                     dt, dist, requiredDist, inRange, eDown, level->takeMaskHold, level->takeMaskHoldRequired);
        }

        if (inRange) {
            if (eDown) {
                level->takeMaskHold += dt;
                if (level->takeMaskHold >= level->takeMaskHoldRequired) {
                    level->maskTaken = true;
                    level->maskTakenMsgTimer = 2.5f;
                }
            } else {
                level->takeMaskHold -= dt * 2.0f;
                if (level->takeMaskHold < 0.0f) level->takeMaskHold = 0.0f;
            }
        } else {
            level->takeMaskHold = 0.0f;
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
