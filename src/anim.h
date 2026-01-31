#ifndef ANIM_H
#define ANIM_H

#include "../raylib/src/raylib.h"
#include <stdbool.h>

typedef struct {
    Texture2D *frames;
    int frame_count;
    float fps;
} AnimClip;

typedef struct {
    AnimClip *clip;
    float time;
    int frame_index;
    bool loop;
} AnimPlayer;

AnimClip LoadAnimClip(const char *directory, float fps);
void UnloadAnimClip(AnimClip *clip);
void AnimPlayer_SetClip(AnimPlayer *player, AnimClip *clip);
void AnimPlayer_Update(AnimPlayer *player, float dt);
Texture2D AnimPlayer_GetFrame(const AnimPlayer *player);

// True when a non-looping player has reached the last frame.
bool AnimPlayer_IsFinished(const AnimPlayer *player);

// Draw the current frame of an AnimPlayer centered at world position with uniform scale.
// This is used for Epilog NPCs.
void DrawAnimPlayer(const AnimPlayer *player, Vector2 position, float scale, Color tint);

#endif // ANIM_H
