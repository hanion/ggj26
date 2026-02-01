#include "../raylib/src/raylib.h"
#include "levels.h"
#include <stdio.h>

typedef enum {
    ED_CLOSED = 0,

    ED_IDLE,

    ED_MOVE_WALL,
    ED_SCALE_WALL,

    ED_MOVE_BG,
    ED_SCALE_BG,

    ED_MOVE_DOOR,
    ED_SCALE_DOOR,

    ED_MOVE_ENEMY,
} EditorState;

const char* EditorState_cstr(EditorState state) {
    switch (state) {
        case ED_CLOSED:     return "ED_CLOSED";
        case ED_IDLE:       return "ED_IDLE";
        case ED_MOVE_WALL:  return "ED_MOVE_WALL";
        case ED_SCALE_WALL: return "ED_SCALE_WALL";
        case ED_MOVE_BG:    return "ED_MOVE_BG";
        case ED_SCALE_BG:   return "ED_SCALE_BG";
        case ED_MOVE_DOOR:  return "ED_MOVE_DOOR";
        case ED_SCALE_DOOR: return "ED_SCALE_DOOR";
        case ED_MOVE_ENEMY: return "ED_MOVE_ENEMY";
	}
    return "INVALID ED STATE";
}


typedef struct {
    Level* level;

    EditorState state;
    int selected;

    Vector2 mouse_world;
    Vector2 drag_offset;

    float move_step;
    float scale_step;
} LevelEditor;


LevelEditor LevelEditor_new(Level* level) {
    return (LevelEditor) {
        .level = level,
        .move_step = 4.0f,
        .scale_step = 4.0f,
    };
}


void editor_pick(LevelEditor* ed) {
    for (int i = 0; i < ed->level->enemyCount; i++) {
        Rectangle r = {0};
        int ext = 25;
        r.x = ed->level->enemies[i].position.x - ext;
        r.y = ed->level->enemies[i].position.y - ext;
        r.width  = 2*ext;
        r.height = 2*ext;
        if (CheckCollisionPointRec(ed->mouse_world, r)) {
            ed->state = ED_MOVE_ENEMY;
            ed->selected = i;
            ed->drag_offset = (Vector2){
                ed->mouse_world.x - ed->level->enemies[i].position.x,
                ed->mouse_world.y - ed->level->enemies[i].position.y
            };
            return;
        }
    }

    for (int i = 0; i < ed->level->doorCount; i++) {
        Rectangle *r = &ed->level->doors[i].rect;
        if (CheckCollisionPointRec(ed->mouse_world, *r)) {
            ed->state = ED_MOVE_DOOR;
            ed->selected = i;
            ed->drag_offset = (Vector2){
                ed->mouse_world.x - r->x,
                ed->mouse_world.y - r->y
            };
            return;
        }
    }

    for (int i = 0; i < ed->level->wallCount; i++) {
        Rectangle *r = &ed->level->walls[i];
        if (CheckCollisionPointRec(ed->mouse_world, *r)) {
            ed->state = ED_MOVE_WALL;
            ed->selected = i;
            ed->drag_offset = (Vector2){
                ed->mouse_world.x - r->x,
                ed->mouse_world.y - r->y
            };
            return;
        }
    }

    for (int i = 0; i < ed->level->bgs_count; i++) {
        Rectangle *r = &ed->level->bgs[i].dest;
        if (CheckCollisionPointRec(ed->mouse_world, *r)) {
            ed->state = ED_MOVE_BG;
            ed->selected = i;
            ed->drag_offset = (Vector2){
                ed->mouse_world.x - r->x,
                ed->mouse_world.y - r->y
            };
            return;
        }
    }
}





void editor_update_state(LevelEditor* ed) {
    if (ed->state == ED_CLOSED) return;

    if (IsMouseButtonPressed(MOUSE_RIGHT_BUTTON)) {
        ed->state = ED_IDLE;
        ed->selected = -1;
    }

    if (IsKeyPressed(KEY_M)) {
        if (ed->state == ED_SCALE_WALL) ed->state = ED_MOVE_WALL;
        if (ed->state == ED_SCALE_BG)   ed->state = ED_MOVE_BG;
    }

    if (IsKeyPressed(KEY_S)) {
        if (ed->state == ED_MOVE_WALL) ed->state = ED_SCALE_WALL;
        if (ed->state == ED_MOVE_BG)   ed->state = ED_SCALE_BG;
    }
}


void editor_update(LevelEditor* ed, Camera2D* camera) {
    if (ed->state == ED_CLOSED) return;


    float mouse_wheel = GetMouseWheelMove();
    camera->zoom += mouse_wheel*0.25f;


    Rectangle* target_rect = NULL;
	Rectangle enemy_dummy_rect = {0};
    switch (ed->state) {
        case ED_MOVE_WALL:
        case ED_SCALE_WALL:   target_rect = &ed->level->walls[ed->selected]; break;
        case ED_MOVE_BG:
        case ED_SCALE_BG:     target_rect = &ed->level->bgs[ed->selected].dest; break;
        case ED_MOVE_DOOR:
        case ED_SCALE_DOOR:   target_rect = &ed->level->doors[ed->selected].rect; break;
        case ED_MOVE_ENEMY: {
            enemy_dummy_rect.x = ed->level->enemies[ed->selected].position.x;
            enemy_dummy_rect.y = ed->level->enemies[ed->selected].position.y;
            target_rect = &enemy_dummy_rect;
        } break;
        default: return;
    }

    if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
        if (ed->state == ED_MOVE_WALL || ed->state == ED_MOVE_BG || ed->state == ED_MOVE_DOOR || ed->state == ED_MOVE_ENEMY) {
            target_rect->x = ed->mouse_world.x - ed->drag_offset.x;
            target_rect->y = ed->mouse_world.y - ed->drag_offset.y;
        } else if (ed->state == ED_SCALE_WALL || ed->state == ED_SCALE_BG || ed->state == ED_SCALE_DOOR) {
            float step = ed->scale_step;
            if (IsKeyDown(KEY_LEFT_SHIFT)) step *= 4.0f;
            if (IsKeyDown(KEY_LEFT_ALT))   step *= 0.25f;

            if (IsKeyDown(KEY_RIGHT)) target_rect->width  += step;
            if (IsKeyDown(KEY_LEFT))  target_rect->width  -= step;
            if (IsKeyDown(KEY_DOWN))  target_rect->height += step;
            if (IsKeyDown(KEY_UP))    target_rect->height -= step;
        }

        if (ed->state == ED_MOVE_ENEMY) {
            ed->level->enemies[ed->selected].position.x = target_rect->x;
            ed->level->enemies[ed->selected].position.y = target_rect->y;
        }
    }
}

const char* PermissionLevel_cstr(PermissionLevel pl) {
    switch (pl) {
        case PERM_NONE:  return "PERM_NONE";
        case PERM_STAFF: return "PERM_STAFF";
        case PERM_GUARD: return "PERM_GUARD";
        case PERM_ADMIN: return "PERM_ADMIN";
	}
    return "INVALID PERM";
}


const char* EnemyType_cstr(EnemyType type) {
    switch (type) {
        case ENEMY_CIVILIAN: return "ENEMY_CIVILIAN";
        case ENEMY_STAFF:    return "ENEMY_STAFF";
        case ENEMY_GUARD:    return "ENEMY_GUARD";
        case ENEMY_ADMIN:    return "ENEMY_ADMIN";
	}
    return "INVALID ENEMY TYPE";
}
const char* AIType_cstr(AIType type) {
    switch (type) {
        case AI_WALKER:   return "AI_WALKER";
        case AI_GUARDIAN: return "AI_WALKER";
	}
    return "INVALID AI TYPE";
}


void level_editor_export(LevelEditor* ed) {
    if (!ed || !ed->level) return;

    Level *level = ed->level;

    printf("// ---- LEVEL EDITOR EXPORT ----\n");
    printf("// ---- WALLS ----\n");
    printf("level->wallCount = %d;\n", level->wallCount);
    for (int i = 0; i < level->wallCount; i++) {
        Rectangle r = level->walls[i];
        printf(
            "level->walls[%d] = (Rectangle){%.0f, %.0f, %.0f, %.0f};\n",
            i, r.x, r.y, r.width, r.height
        );
    }

    printf("\n// ---- BACKGROUNDS ----\n");
    printf("level->bgs_count = %zu;\n", level->bgs_count);
    printf("static Texture2D level_bg_textures[%zu];\n", level->bgs_count);
    for (int i = 0; i < level->bgs_count; i++) {
        printf("if (0 == level_bg_textures[%d].id) level_bg_textures[%d] = LoadTexture(\"TEXTURE_PATH_HERE\");\n", i, i);
    }
    for (int i = 0; i < level->bgs_count; i++) {
        Background *b = &level->bgs[i];
        printf("level->bgs[%d] = (Background){level_bg_textures[%d], (Rectangle){%.0f, %.0f, %.0f, %.0f}, (Rectangle){%.0f, %.0f, %.0f, %.0f}};\n",
            i, i, b->source.x, b->source.y, b->source.width, b->source.height,
            b->dest.x, b->dest.y, b->dest.width, b->dest.height
        );
    }

    printf("\n// ---- DOORS ----\n");
    printf("level->doorCount = %d;\n", level->doorCount);
    for (int i = 0; i < level->doorCount; i++) {
        Door *d = &level->doors[i];

        printf("level->doors[%d].rect = (Rectangle){%f,%f,%f,%f};\n",
            i, d->rect.x,  d->rect.y, d->rect.width, d->rect.height
        );

        printf("level->doors[%d].requiredPerm = %s;\n",
            i, PermissionLevel_cstr(d->requiredPerm)
        );
    }


    printf("\n// ---- ENEMIES ----\n");
    printf("level->enemyCount = %d;\n", level->enemyCount);
    for (int i = 0; i < level->enemyCount; i++) {
        Entity *e = &level->enemies[i];

        printf("Identity enemy_id_%d = {.permissionLevel = PERM_ADMIN, .color = PURPLE, .speed = 250.0f};\n", i);
        printf("level->enemies[%d] = InitEnemy((Vector2){%f,%f}, %s);\n",
            i, e->position.x, e->position.y, EnemyType_cstr(e->type)
        );
        printf("level->enemies[%d].identity = enemy_id_%d;\n", i, i);
    }

    printf("// ---- LEVEL EDITOR EXPORT END ----\n");
}




void level_editor_draw(LevelEditor* ed, Camera2D* camera) {
    if (!ed || !ed->level) return;

    if (IsKeyPressed(KEY_O)) {
        ed->state = (ed->state == ED_CLOSED) ? ED_IDLE : ED_CLOSED;
        ed->selected = -1;
    }

    if (ed->state == ED_CLOSED)  return;


    ed->mouse_world = GetScreenToWorld2D(GetMousePosition(), *camera);
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        editor_pick(ed);
    }
    editor_update_state(ed);
    editor_update(ed, camera);

    if (IsKeyPressed(KEY_I)) {
        level_editor_export(ed);
    }

    BeginMode2D(*camera);
    // draw wall
    for (int i = 0; i < ed->level->wallCount; i++) {
        DrawRectangleRec(ed->level->walls[i], DARKGRAY);

        if ((ed->state == ED_MOVE_WALL || ed->state == ED_SCALE_WALL) &&
            i == ed->selected) {
            DrawRectangleLinesEx(ed->level->walls[i], 2, RED);
        }
    }
    // draw bg
    for (int i = 0; i < ed->level->bgs_count; i++) {
        if ((ed->state == ED_MOVE_BG || ed->state == ED_SCALE_BG) &&
            i == ed->selected) {
            DrawRectangleLinesEx(ed->level->bgs[i].dest, 2, RED);
        }
    }
    for (int i = 0; i < ed->level->enemyCount; i++) {
        int ext = 25;
        if ((ed->state == ED_MOVE_ENEMY) &&
            i == ed->selected) {
            DrawCircleLines(ed->level->enemies[i].position.x, ed->level->enemies[i].position.y, 25, RED);
            DrawCircleLines(ed->level->enemies[i].position.x, ed->level->enemies[i].position.y, 24, RED);
            DrawCircleLines(ed->level->enemies[i].position.x, ed->level->enemies[i].position.y, 23, RED);
        }
    }


    EndMode2D();
    DrawText(
        TextFormat(
            "EDITOR | state: %s | selected: %d",
            EditorState_cstr(ed->state),
            ed->selected
        ),
        10, 800, 20, YELLOW
    );
}


