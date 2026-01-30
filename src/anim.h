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
} AnimPlayer;

AnimClip LoadAnimClip(const char *directory, float fps);
void UnloadAnimClip(AnimClip *clip);
void AnimPlayer_SetClip(AnimPlayer *player, AnimClip *clip);
void AnimPlayer_Update(AnimPlayer *player, float dt);
Texture2D AnimPlayer_GetFrame(const AnimPlayer *player);

#endif // ANIM_H
