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
} EditorState;


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
    camera->zoom += mouse_wheel;


    Rectangle* target_rect = NULL;
    switch (ed->state) {
        case ED_MOVE_WALL:
        case ED_SCALE_WALL:   target_rect = &ed->level->walls[ed->selected]; break;
        case ED_MOVE_BG:
        case ED_SCALE_BG:     target_rect = &ed->level->bgs[ed->selected].dest; break;
        case ED_MOVE_DOOR:
        case ED_SCALE_DOOR:   target_rect = &ed->level->doors[ed->selected].rect; break;
        default: return;
    }

    if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
        if (ed->state == ED_MOVE_WALL || ed->state == ED_MOVE_BG || ed->state == ED_MOVE_DOOR) {
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
    }
}


void level_editor_export(LevelEditor* ed) {
    if (!ed || !ed->level) return;

    Level *level = ed->level;

    printf("// ---- LEVEL EDITOR EXPORT ----\n");
    printf("// ---- WALLS ----\n");
    for (int i = 0; i < level->wallCount; i++) {
        Rectangle r = level->walls[i];
        printf(
            "level->walls[level->wallCount++] = (Rectangle){%.0f, %.0f, %.0f, %.0f};\n",
            r.x, r.y, r.width, r.height
        );
    }

    printf("\n// ---- BACKGROUNDS ----\n");
    for (int i = 0; i < level->bgs_count; i++) {
        Background *b = &level->bgs[i];
        printf(
            "level->bgs[level->bgs_count++] = (Background){TEXTURENAMEHERE, (Rectangle){%.0f, %.0f, %.0f, %.0f}, (Rectangle){%.0f, %.0f, %.0f, %.0f}};\n",
            b->source.x, b->source.y, b->source.width, b->source.height,
            b->dest.x, b->dest.y, b->dest.width, b->dest.height
        );
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
    EndMode2D();
    DrawText(
        TextFormat(
            "EDITOR | state: %d | selected: %d",
            ed->state,
            ed->selected
        ),
        10, 800, 20, YELLOW
    );
}


