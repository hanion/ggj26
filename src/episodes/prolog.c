// filepath: /home/noobuntu/ggj26/src/episodes/prolog.c
#include "../levels.h"
#include "../types.h"
#include "../npc/npc.h"

static Texture2D texProlog;
static Texture2D texBalikci[3]; // shared frames for balikci
static Texture2D texKiz[4];     // shared frames for kiz_cocuk
static Texture2D texSigaraci[6];// shared frames for sigaraci

extern bool gameWon;
static const char *dlgBalikci[] = {
    "Sen gereksiz insan yine mi?.",
    "Burada istenmiyorsun., Kimligin bile yok",
    "No time."
};

static const char *dlgKiz[] = {
    "YOK OL BURADAN!",
    "Igrencsin! Kimsen yok! Burada istenmiyorsun",
    "...." 
};

static const char *dlgSigaraci[] = {
    "S*ktr git buradan",
    "YURU DEDIM SANA LAN!.",
    "Igrent",
    "Take the mask."
};

static const char *plBalikci[] = {
    "Lutfen benimle konus.",
    "Kizmayin bana sadece yemek istiyorum.",
    "Ozur dilerim."
};

static const char *plKiz[] = {
    "Kimligim olmayabilir ama beni dislama",
    "Neden bana boyle davraniyorsunuz",
    "Ozur dilerim."
};

static const char *plSigaraci[] = {
    "Arkadas olalim mi",
    "Bana boyle davranma",
    "BANA BOYLE DAVRANMA DEDIM SANA!.",
    "Why?"
};

void InitProlog(Level *level) {
    level->id = 0;
    level->playerSpawn = (Vector2){ 400, 800 };

    // Set a default Identity for the starting player
    level->playerStartId = (Identity){0};
    level->playerStartId.color = (Color){200, 200, 255, 255};
    level->playerStartId.speed = 200.0f;

    // Load map texture, with a fallback if missing
    const char *mapPath = "assets/prolog/prolog_map.png";
    if (FileExists(mapPath)) {
        texProlog = LoadTexture(mapPath);
    } else {
        TraceLog(LOG_WARNING, "Prolog map not found at %s. Using generated placeholder.", mapPath);
        Image img = GenImageColor(1920, 1080, (Color){30, 30, 35, 255});
        // Simple grid overlay
        for (int y = 0; y < 1080; y += 64) {
            for (int x = 0; x < 1920; x++) {
                ImageDrawPixel(&img, x, y, (Color){60, 60, 70, 255});
            }
        }
        for (int x = 0; x < 1920; x += 64) {
            for (int y = 0; y < 1080; y++) {
                ImageDrawPixel(&img, x, y, (Color){60, 60, 70, 255});
            }
        }
        texProlog = LoadTextureFromImage(img);
        UnloadImage(img);
    }

    // Load shared NPC frames once
    const char *base = "assets/street_animation";
    // balikci (3 frames)
    texBalikci[0] = LoadTexture(TextFormat("%s/balikci/1.png", base));
    texBalikci[1] = LoadTexture(TextFormat("%s/balikci/2.png", base));
    texBalikci[2] = LoadTexture(TextFormat("%s/balikci/3.png", base));
    // kiz_cocuk (4 frames)
    texKiz[0] = LoadTexture(TextFormat("%s/kiz_cocuk/1.png", base));
    texKiz[1] = LoadTexture(TextFormat("%s/kiz_cocuk/2.png", base));
    texKiz[2] = LoadTexture(TextFormat("%s/kiz_cocuk/3.png", base));
    texKiz[3] = LoadTexture(TextFormat("%s/kiz_cocuk/4.png", base));
    // sigaraci (6 frames)
    texSigaraci[0] = LoadTexture(TextFormat("%s/sigaraci/1.png", base));
    texSigaraci[1] = LoadTexture(TextFormat("%s/sigaraci/2.png", base));
    texSigaraci[2] = LoadTexture(TextFormat("%s/sigaraci/3.png", base));
    texSigaraci[3] = LoadTexture(TextFormat("%s/sigaraci/4.png", base));
    texSigaraci[4] = LoadTexture(TextFormat("%s/sigaraci/5.png", base));
    texSigaraci[5] = LoadTexture(TextFormat("%s/sigaraci/6.png", base));


    level->bgs_count = 1;
    level->bgs[0].texture = texProlog;
    level->bgs[0].source = (Rectangle){0, 0, texProlog.width, texProlog.height};
    level->bgs[0].dest = (Rectangle){0, 0, texProlog.width, texProlog.height};

    level->wallCount = 0;
    level->doorCount = 0;
    level->enemyCount = 0;

    // --- NPCs ---
    level->npcCount = 3; // one of each type
    // positions for each distinct NPC
    Vector2 npcPos[3] = { { 600, 820 }, { 800, 800 }, { 500, 760 } };
    const char *names[3] = { "Balikci", "Kiz", "Sigaraci" };

    // Balikci (index 0)
    level->npcs[0].position = npcPos[0];
    level->npcs[0].scale = 0.50f;
    level->npcs[0].radius = 24.0f;
    level->npcs[0].tint = WHITE;
    level->npcs[0].name = names[0];
    level->npcs[0].frameDuration = 0.35f; // faster
    level->npcs[0].frameIndex = 0;
    level->npcs[0].frameTimer = 0.0f;
    level->npcs[0].frameCount = 3;
    level->npcs[0].frames[0] = texBalikci[0];
    level->npcs[0].frames[1] = texBalikci[1];
    level->npcs[0].frames[2] = texBalikci[2];
    level->npcs[0].dialogueLines = dlgBalikci;
    level->npcs[0].dialogueLineCount = (int)(sizeof(dlgBalikci) / sizeof(dlgBalikci[0]));
    level->npcs[0].dialogueLineIndex = 0;
    level->npcs[0].dialogueCompleted = false;
    level->npcs[0].playerReplyLines = plBalikci;
    level->npcs[0].playerReplyLineCount = (int)(sizeof(plBalikci) / sizeof(plBalikci[0]));

    // Kiz (index 1)
    level->npcs[1].position = npcPos[1];
    level->npcs[1].scale = 0.3f;
    level->npcs[1].radius = 22.0f;
    level->npcs[1].tint = WHITE;
    level->npcs[1].name = names[1];
    level->npcs[1].frameDuration = 0.35f; // medium speed
    level->npcs[1].frameIndex = 0;
    level->npcs[1].frameTimer = 0.0f;
    level->npcs[1].frameCount = 4;
    level->npcs[1].frames[0] = texKiz[0];
    level->npcs[1].frames[1] = texKiz[1];
    level->npcs[1].frames[2] = texKiz[2];
    level->npcs[1].frames[3] = texKiz[3];
    level->npcs[1].dialogueLines = dlgKiz;
    level->npcs[1].dialogueLineCount = (int)(sizeof(dlgKiz) / sizeof(dlgKiz[0]));
    level->npcs[1].dialogueLineIndex = 0;
    level->npcs[1].dialogueCompleted = false;
    level->npcs[1].playerReplyLines = plKiz;
    level->npcs[1].playerReplyLineCount = (int)(sizeof(plKiz) / sizeof(plKiz[0]));

    // Sigaraci (index 2)
    level->npcs[2].position = npcPos[2];
    level->npcs[2].scale = 0.50f;
    level->npcs[2].radius = 24.0f;
    level->npcs[2].tint = WHITE;
    level->npcs[2].name = names[2];
    level->npcs[2].frameDuration = 0.60f; // slowest
    level->npcs[2].frameIndex = 0;
    level->npcs[2].frameTimer = 0.0f;
    level->npcs[2].frameCount = 6;
    level->npcs[2].frames[0] = texSigaraci[0];
    level->npcs[2].frames[1] = texSigaraci[1];
    level->npcs[2].frames[2] = texSigaraci[2];
    level->npcs[2].frames[3] = texSigaraci[3];
    level->npcs[2].frames[4] = texSigaraci[4];
    level->npcs[2].frames[5] = texSigaraci[5];
    level->npcs[2].dialogueLines = dlgSigaraci;
    level->npcs[2].dialogueLineCount = (int)(sizeof(dlgSigaraci) / sizeof(dlgSigaraci[0]));
    level->npcs[2].dialogueLineIndex = 0;
    level->npcs[2].dialogueCompleted = false;
    level->npcs[2].playerReplyLines = plSigaraci;
    level->npcs[2].playerReplyLineCount = (int)(sizeof(plSigaraci) / sizeof(plSigaraci[0]));

    // Ensure dialogue/UI defaults
    level->activeDialogueText = NULL;
    level->activeDialogueTimer = 0.0f;
    level->activeDialoguePos = (Vector2){0, 0};
    level->activeDialogueIsPlayer = false;
    level->showOutroLine = false;
    level->outroLineTimer = 0.0f;
}

void UnloadProlog() {
    // Unload background and shared NPC textures
    if (texProlog.id) UnloadTexture(texProlog);
    for (int f=0; f<3; f++) {
        if (texBalikci[f].id) UnloadTexture(texBalikci[f]);
    }
    for (int f = 0; f < 4; f++) {
        if (texKiz[f].id) UnloadTexture(texKiz[f]);
    }
    for (int f = 0; f < 6; f++) {
        if (texSigaraci[f].id) UnloadTexture(texSigaraci[f]);
    }
}
