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
    if (IsKeyPressed(KEY_O)) {
        ed->state = (ed->state == ED_CLOSED) ? ED_IDLE : ED_CLOSED;
        ed->selected = -1;
    }
    if (ed->state == ED_CLOSED) {
        return;
    }

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


void editor_update(LevelEditor* ed) {
    switch (ed->state) {

        case ED_MOVE_WALL: {
            if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
                Rectangle *r = &ed->level->walls[ed->selected];
                r->x = ed->mouse_world.x - ed->drag_offset.x;
                r->y = ed->mouse_world.y - ed->drag_offset.y;
            }
        } break;

        case ED_SCALE_WALL: {
            Rectangle *r = &ed->level->walls[ed->selected];
            float step = ed->scale_step;
            if (IsKeyDown(KEY_LEFT_SHIFT)) step *= 4.0f;
            if (IsKeyDown(KEY_LEFT_ALT))   step *= 0.25f;

            if (IsKeyDown(KEY_RIGHT)) r->width  += step;
            if (IsKeyDown(KEY_LEFT))  r->width  -= step;
            if (IsKeyDown(KEY_DOWN))  r->height += step;
            if (IsKeyDown(KEY_UP))    r->height -= step;
        } break;

        case ED_MOVE_BG: {
            if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
                Background *b = &ed->level->bgs[ed->selected];
                b->dest.x = ed->mouse_world.x - ed->drag_offset.x;
                b->dest.y = ed->mouse_world.y - ed->drag_offset.y;
            }
        } break;

        case ED_SCALE_BG: {
            Background *b = &ed->level->bgs[ed->selected];
            float step = ed->scale_step;
            if (IsKeyDown(KEY_LEFT_SHIFT)) step *= 4.0f;
            if (IsKeyDown(KEY_LEFT_ALT))   step *= 0.25f;

            if (IsKeyDown(KEY_RIGHT)) b->dest.width  += step;
            if (IsKeyDown(KEY_LEFT))  b->dest.width  -= step;
            if (IsKeyDown(KEY_DOWN))  b->dest.height += step;
            if (IsKeyDown(KEY_UP))    b->dest.height -= step;
        } break;

        default: break;
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




void level_editor_draw(LevelEditor* ed, Camera2D camera) {
    if (!ed || !ed->level) return;

    ed->mouse_world = GetScreenToWorld2D(GetMousePosition(), camera);

    BeginMode2D(camera);

    if (ed->state != ED_CLOSED) {
        if (IsKeyPressed(KEY_I)) {
            level_editor_export(ed);
        }

        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            editor_pick(ed);
        }


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
    }


    editor_update_state(ed);
    editor_update(ed);

    EndMode2D();

    if (ed->state != ED_CLOSED) {
        DrawText(
            TextFormat(
                "EDITOR | state: %d | selected: %d",
                ed->state,
                ed->selected
            ),
            10, 800, 20, YELLOW
        );
    }
}


