#include "anim.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// OS-independent directory scanning (pure C):
// - POSIX: opendir/readdir + stat
// - Windows: FindFirstFile/FindNextFile
#if defined(_WIN32)
    #define WIN32_LEAN_AND_MEAN
    #include <windows.h>
#else
    #include <dirent.h>
    #include <sys/stat.h>
#endif

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

static bool IsRegularFileInDir(const char *directory, const char *name) {
    if (!directory || !name) return false;

    // Build "directory/name" (POSIX) or "directory\\name" (Windows) for stat.
    // We still accept forward slashes on Windows, but backslash is conventional.
#if defined(_WIN32)
    const char sep = '\\';
#else
    const char sep = '/';
#endif

    size_t dirLen = strlen(directory);
    size_t nameLen = strlen(name);
    bool needsSep = (dirLen > 0 && directory[dirLen - 1] != '/' && directory[dirLen - 1] != '\\');
    size_t pathLen = dirLen + (needsSep ? 1 : 0) + nameLen + 1;

    char *path = (char *)malloc(pathLen);
    if (!path) return false;

    if (needsSep) {
        snprintf(path, pathLen, "%s%c%s", directory, sep, name);
    } else {
        snprintf(path, pathLen, "%s%s", directory, name);
    }

#if defined(_WIN32)
    DWORD attr = GetFileAttributesA(path);
    bool ok = (attr != INVALID_FILE_ATTRIBUTES) && ((attr & FILE_ATTRIBUTE_DIRECTORY) == 0);
#else
    struct stat st;
    bool ok = (stat(path, &st) == 0) && S_ISREG(st.st_mode);
#endif

    free(path);
    return ok;
}

AnimClip LoadAnimClip(const char *directory, float fps) {
    AnimClip clip = {0};
    clip.fps = fps;

    size_t capacity = 0;
    int count = 0;
    char **names = NULL;

#if defined(_WIN32)
    // Windows directory scan: directory\*.png
    const char *patternSuffix = "*.png";
    size_t dirLen = strlen(directory);
    bool needsSep = (dirLen > 0 && directory[dirLen - 1] != '/' && directory[dirLen - 1] != '\\');
    size_t patternLen = dirLen + (needsSep ? 1 : 0) + strlen(patternSuffix) + 1;
    char *pattern = (char *)malloc(patternLen);
    if (!pattern) {
        TraceLog(LOG_ERROR, "Failed to allocate animation search pattern");
        return clip;
    }
    if (needsSep) {
        snprintf(pattern, patternLen, "%s\\%s", directory, patternSuffix);
    } else {
        snprintf(pattern, patternLen, "%s%s", directory, patternSuffix);
    }

    WIN32_FIND_DATAA ffd;
    HANDLE hFind = FindFirstFileA(pattern, &ffd);
    free(pattern);
    if (hFind == INVALID_HANDLE_VALUE) {
        TraceLog(LOG_ERROR, "Failed to open animation dir: %s", directory);
        return clip;
    }

    do {
        const char *name = ffd.cFileName;
        if (!HasPngExtension(name)) continue;
        if (!IsRegularFileInDir(directory, name)) continue;

        if (count >= (int)capacity) {
            size_t nextCapacity = capacity == 0 ? 8 : capacity * 2;
            char **nextNames = (char **)realloc(names, nextCapacity * sizeof(char *));
            if (!nextNames) {
                TraceLog(LOG_ERROR, "Failed to allocate animation names");
                break;
            }
            names = nextNames;
            capacity = nextCapacity;
        }

        size_t nameLen = strlen(name) + 1;
        names[count] = (char *)malloc(nameLen);
        if (!names[count]) {
            TraceLog(LOG_ERROR, "Failed to allocate animation name");
            break;
        }
        memcpy(names[count], name, nameLen);
        count++;
    } while (FindNextFileA(hFind, &ffd) != 0);

    FindClose(hFind);
#else
    DIR *dir = opendir(directory);
    if (!dir) {
        TraceLog(LOG_ERROR, "Failed to open animation dir: %s", directory);
        return clip;
    }

    struct dirent *entry = NULL;
    while ((entry = readdir(dir)) != NULL) {
        const char *name = entry->d_name;
        // Skip . and ..
        if (strcmp(name, ".") == 0 || strcmp(name, "..") == 0) continue;
        if (!HasPngExtension(name)) continue;
        if (!IsRegularFileInDir(directory, name)) continue;

        if (count >= (int)capacity) {
            size_t nextCapacity = capacity == 0 ? 8 : capacity * 2;
            char **nextNames = (char **)realloc(names, nextCapacity * sizeof(char *));
            if (!nextNames) {
                TraceLog(LOG_ERROR, "Failed to allocate animation names");
                break;
            }
            names = nextNames;
            capacity = nextCapacity;
        }

        size_t nameLen = strlen(name) + 1;
        names[count] = (char *)malloc(nameLen);
        if (!names[count]) {
            TraceLog(LOG_ERROR, "Failed to allocate animation name");
            break;
        }
        memcpy(names[count], name, nameLen);
        count++;
    }

    closedir(dir);
#endif

    if (count == 0) {
        TraceLog(LOG_ERROR, "No animation frames found in %s", directory);
    free(names);
    return clip;
    }

    qsort(names, count, sizeof(char *), CompareFrameNames);

    clip.frames = (Texture2D *)malloc(sizeof(Texture2D) * (size_t)count);
    if (!clip.frames) {
        TraceLog(LOG_ERROR, "Failed to allocate animation frames");
        for (int i = 0; i < count; i++) free(names[i]);
        free(names);
        return clip;
    }

    for (int i = 0; i < count; i++) {
        size_t pathLen = strlen(directory) + strlen(names[i]) + 2;
    char *path = (char *)malloc(pathLen);
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
    // Default behavior: loop animations unless caller disables it.
    player->loop = true;
}

void AnimPlayer_Update(AnimPlayer *player, float dt) {
    if (!player || !player->clip || player->clip->frame_count == 0) return;
    player->time += dt;
    float frameTime = 1.0f / player->clip->fps;
    while (player->time >= frameTime) {
        player->time -= frameTime;
        if (player->loop) {
            player->frame_index = (player->frame_index + 1) % player->clip->frame_count;
        } else {
            if (player->frame_index < player->clip->frame_count - 1) {
                player->frame_index++;
            } else {
                // Clamp on last frame.
                player->frame_index = player->clip->frame_count - 1;
                player->time = 0.0f;
                break;
            }
        }
    }
}

Texture2D AnimPlayer_GetFrame(const AnimPlayer *player) {
    if (!player || !player->clip || player->clip->frame_count == 0) {
        return (Texture2D){0};
    }
    return player->clip->frames[player->frame_index];
}

bool AnimPlayer_IsFinished(const AnimPlayer *player) {
    if (!player || !player->clip || player->clip->frame_count == 0) return true;
    if (player->loop) return false;
    return player->frame_index >= (player->clip->frame_count - 1);
}
