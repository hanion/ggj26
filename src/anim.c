#include "anim.h"

#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static bool HasPngExtension(const char *name) {
    const char *ext = strrchr(name, '.');
    if (!ext) return false;
    return strcmp(ext, ".png") == 0;
}

static int CompareFrameNames(const void *a, const void *b) {
    const char *nameA = *(const char **)a;
    const char *nameB = *(const char **)b;
    int indexA = 0;
    int indexB = 0;
    sscanf(nameA, "%*[^0-9]%d", &indexA);
    sscanf(nameB, "%*[^0-9]%d", &indexB);
    if (indexA < indexB) return -1;
    if (indexA > indexB) return 1;
    return strcmp(nameA, nameB);
}

AnimClip LoadAnimClip(const char *directory, float fps) {
    AnimClip clip = {0};
    clip.fps = fps;

    DIR *dir = opendir(directory);
    if (!dir) {
        TraceLog(LOG_ERROR, "Failed to open animation dir: %s", directory);
        return clip;
    }

    size_t capacity = 0;
    int count = 0;
    char **names = NULL;

    struct dirent *entry = NULL;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_DIR) continue;
        if (!HasPngExtension(entry->d_name)) continue;

        if (count >= (int)capacity) {
            size_t nextCapacity = capacity == 0 ? 8 : capacity * 2;
            char **nextNames = realloc(names, nextCapacity * sizeof(char *));
            if (!nextNames) {
                TraceLog(LOG_ERROR, "Failed to allocate animation names");
                break;
            }
            names = nextNames;
            capacity = nextCapacity;
        }

        size_t nameLen = strlen(entry->d_name) + 1;
        names[count] = malloc(nameLen);
        if (!names[count]) {
            TraceLog(LOG_ERROR, "Failed to allocate animation name");
            break;
        }
        memcpy(names[count], entry->d_name, nameLen);
        count++;
    }

    closedir(dir);

    if (count == 0) {
        TraceLog(LOG_ERROR, "No animation frames found in %s", directory);
        free(names);
        return clip;
    }

    qsort(names, count, sizeof(char *), CompareFrameNames);

    clip.frames = malloc(sizeof(Texture2D) * count);
    if (!clip.frames) {
        TraceLog(LOG_ERROR, "Failed to allocate animation frames");
        for (int i = 0; i < count; i++) free(names[i]);
        free(names);
        return clip;
    }

    for (int i = 0; i < count; i++) {
        size_t pathLen = strlen(directory) + strlen(names[i]) + 2;
        char *path = malloc(pathLen);
        if (!path) {
            TraceLog(LOG_ERROR, "Failed to allocate frame path");
            clip.frames[i] = (Texture2D){0};
            free(names[i]);
            continue;
        }
        snprintf(path, pathLen, "%s/%s", directory, names[i]);
        clip.frames[i] = LoadTexture(path);
        if (clip.frames[i].id == 0) {
            TraceLog(LOG_ERROR, "Failed to load frame: %s", path);
        }
        free(path);
        free(names[i]);
    }

    free(names);
    clip.frame_count = count;
    return clip;
}

void UnloadAnimClip(AnimClip *clip) {
    if (!clip || !clip->frames) return;
    for (int i = 0; i < clip->frame_count; i++) {
        if (clip->frames[i].id != 0) {
            UnloadTexture(clip->frames[i]);
        }
    }
    free(clip->frames);
    clip->frames = NULL;
    clip->frame_count = 0;
}

void AnimPlayer_SetClip(AnimPlayer *player, AnimClip *clip) {
    if (!player || player->clip == clip) return;
    player->clip = clip;
    player->time = 0.0f;
    player->frame_index = 0;
}

void AnimPlayer_Update(AnimPlayer *player, float dt) {
    if (!player || !player->clip || player->clip->frame_count == 0) return;
    player->time += dt;
    float frameTime = 1.0f / player->clip->fps;
    while (player->time >= frameTime) {
        player->time -= frameTime;
        player->frame_index = (player->frame_index + 1) % player->clip->frame_count;
    }
}

Texture2D AnimPlayer_GetFrame(const AnimPlayer *player) {
    if (!player || !player->clip || player->clip->frame_count == 0) {
        return (Texture2D){0};
    }
    return player->clip->frames[player->frame_index];
}
