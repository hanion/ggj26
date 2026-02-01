#include "editor.h"
#include <stdio.h>
#include <string.h>
#include "../raylib/src/rlgl.h"
#include "enemies/enemy.h"
#include "gameplay_helpers.h"


bool is_state_closed(EditorState state) { return state == ED_CLOSED; }
bool is_state_idle(EditorState state)   { return state == ED_IDLE; }

bool is_state_wall(EditorState state) {
    return state == ED_MOVE_WALL
        || state == ED_SCALE_WALL
        || state == ED_ROTATE_WALL;
}
bool is_state_bg(EditorState state) {
    return state == ED_MOVE_BG
        || state == ED_SCALE_BG;
}
bool is_state_door(EditorState state) {
    return state == ED_MOVE_DOOR
        || state == ED_SCALE_DOOR;
}
bool is_state_enemy(EditorState state) {
    return state == ED_MOVE_ENEMY;
}
bool is_state_wa(EditorState state) {
    return state == ED_MOVE_WA
        || state == ED_SCALE_WA;
}


bool is_state_move(EditorState state) {
    return state == ED_MOVE_WALL
        || state == ED_MOVE_BG
        || state == ED_MOVE_DOOR
        || state == ED_MOVE_WA
        || state == ED_MOVE_ENEMY;
}
bool is_state_scale(EditorState state) {
    return state == ED_SCALE_WALL
        || state == ED_SCALE_BG
        || state == ED_SCALE_WA
        || state == ED_SCALE_DOOR;
}
bool is_state_rotate(EditorState state) {
    return state == ED_ROTATE_WALL;
}


const char* EditorState_cstr(EditorState state) {
    switch (state) {
        case ED_CLOSED:      return "CLOSED";
        case ED_IDLE:        return "IDLE";
        case ED_MOVE_WALL:   return "MOVE_WALL";
        case ED_SCALE_WALL:  return "SCALE_WALL";
        case ED_ROTATE_WALL: return "ROTATE_WALL";
        case ED_MOVE_BG:     return "MOVE_BG";
        case ED_SCALE_BG:    return "SCALE_BG";
        case ED_MOVE_DOOR:   return "MOVE_DOOR";
        case ED_SCALE_DOOR:  return "SCALE_DOOR";
        case ED_MOVE_ENEMY:  return "MOVE_ENEMY";
        case ED_MOVE_WA:     return "MOVE_WA";
        case ED_SCALE_WA:    return "SCALE_WA";
    }
    return "INVALID ED STATE";
}


LevelEditor LevelEditor_new(Level* level) {
    return (LevelEditor) {
        .level = level,
        .move_step = 4.0f,
        .scale_step = 4.0f,
    };
}


void editor_pick(LevelEditor* ed) {
    Rectangle *r = &ed->level->win_area;
    if (CheckCollisionPointRec(ed->mouse_world, *r)) {
        ed->state = ED_MOVE_WA;
        ed->selected = -2;
        ed->drag_offset = (Vector2){
            ed->mouse_world.x - r->x,
            ed->mouse_world.y - r->y
        };
        return;
    }

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
        Wall w = ed->level->walls[i];
        if (CheckCollisionCircleRotatedRect(ed->mouse_world, 2.0f, w.rect, w.rotation)) {
            ed->state = ED_MOVE_WALL;
            ed->selected = i;
            ed->drag_offset = (Vector2){
                ed->mouse_world.x - w.rect.x,
                ed->mouse_world.y - w.rect.y
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
        if (is_state_wall(ed->state)) ed->state = ED_MOVE_WALL;
        if (is_state_door(ed->state)) ed->state = ED_MOVE_DOOR;
        if (is_state_bg(ed->state))   ed->state = ED_MOVE_BG;
        if (is_state_wa(ed->state))   ed->state = ED_MOVE_WA;
    }

    if (IsKeyPressed(KEY_S)) {
        if (is_state_wall(ed->state)) ed->state = ED_SCALE_WALL;
        if (is_state_door(ed->state)) ed->state = ED_SCALE_DOOR;
        if (is_state_bg(ed->state))   ed->state = ED_SCALE_BG;
        if (is_state_wa(ed->state))   ed->state = ED_SCALE_WA;
    }

    if (IsKeyPressed(KEY_R)) {
        if (is_state_wall(ed->state)) ed->state = ED_ROTATE_WALL;
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
        case ED_ROTATE_WALL:
        case ED_SCALE_WALL:   target_rect = &ed->level->walls[ed->selected].rect; break;
        case ED_MOVE_BG:
        case ED_SCALE_BG:     target_rect = &ed->level->bgs[ed->selected].dest; break;
        case ED_MOVE_DOOR:
        case ED_SCALE_DOOR:   target_rect = &ed->level->doors[ed->selected].rect; break;
        case ED_MOVE_WA:
        case ED_SCALE_WA:     target_rect = &ed->level->win_area; break;
        case ED_MOVE_ENEMY: {
            enemy_dummy_rect.x = ed->level->enemies[ed->selected].position.x;
            enemy_dummy_rect.y = ed->level->enemies[ed->selected].position.y;
            target_rect = &enemy_dummy_rect;
        } break;
        default: return;
    }

    if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
        if (is_state_move(ed->state)) {
            target_rect->x = ed->mouse_world.x - ed->drag_offset.x;
            target_rect->y = ed->mouse_world.y - ed->drag_offset.y;
        } 
    }

    if (is_state_move(ed->state)) {
        float step = ed->move_step;
        if (IsKeyDown(KEY_RIGHT)) target_rect->x  += step;
        if (IsKeyDown(KEY_LEFT))  target_rect->x  -= step;
        if (IsKeyDown(KEY_DOWN))  target_rect->y += step;
        if (IsKeyDown(KEY_UP))    target_rect->y -= step;
    }


    if (is_state_scale(ed->state)) {
        float step = ed->scale_step;
        if (IsKeyDown(KEY_LEFT_SHIFT)) step *= 4.0f;
        if (IsKeyDown(KEY_LEFT_ALT))   step *= 0.25f;

        if (IsKeyDown(KEY_RIGHT)) target_rect->width  += step;
        if (IsKeyDown(KEY_LEFT))  target_rect->width  -= step;
        if (IsKeyDown(KEY_DOWN))  target_rect->height += step;
        if (IsKeyDown(KEY_UP))    target_rect->height -= step;
    }

    if (is_state_rotate(ed->state)) {
        float rotStep = 1.0f;
        if (IsKeyDown(KEY_LEFT_SHIFT)) rotStep = 5.0f;
        if (IsKeyDown(KEY_RIGHT)) ed->level->walls[ed->selected].rotation += rotStep;
        if (IsKeyDown(KEY_LEFT))  ed->level->walls[ed->selected].rotation -= rotStep;
    }

    // apply rect edit to enemy
    // because enemy does not have rect
    if (ed->state == ED_MOVE_ENEMY) {
        ed->level->enemies[ed->selected].position.x = target_rect->x;
        ed->level->enemies[ed->selected].position.y = target_rect->y;
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
        Rectangle r = level->walls[i].rect;
        float rot = level->walls[i].rotation;
        printf(
            "level->walls[%d] = (Wall){(Rectangle){%.0f, %.0f, %.0f, %.0f}, %.2ff};\n",
            i, r.x, r.y, r.width, r.height, rot
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

    printf("\n// ---- WIN AREA ----\n");
    printf("level->win_area = (Rectangle){%f,%f,%f,%f};\n",
           level->win_area.x,
           level->win_area.y,
           level->win_area.width,
           level->win_area.height
    );

    printf("// ---- LEVEL EDITOR EXPORT END ----\n");
}


void editor_draw_debug(LevelEditor* ed, Camera2D* camera) {
    BeginMode2D(*camera);
    // draw wall
    for (int i = 0; i < ed->level->wallCount; i++) {
        Wall* w = &ed->level->walls[i];
        Vector2 center = { w->rect.x + w->rect.width/2.0f, w->rect.y + w->rect.height/2.0f };
        DrawRectanglePro(
            (Rectangle){center.x, center.y, w->rect.width, w->rect.height},
            (Vector2){w->rect.width/2.0f, w->rect.height/2.0f},
            w->rotation, 
            DARKGRAY
        );

        if (is_state_wall(ed->state) && i == ed->selected) {
             rlPushMatrix();
             rlTranslatef(center.x, center.y, 0);
             rlRotatef(w->rotation, 0,0,1);
             rlTranslatef(-w->rect.width/2.0f, -w->rect.height/2.0f, 0);
             DrawRectangleLines(0, 0, w->rect.width, w->rect.height, RED);
             rlPopMatrix();
        }
    }
    // draw bg
    for (int i = 0; i < ed->level->bgs_count; i++) {
        if (is_state_bg(ed->state) && i == ed->selected) {
            DrawRectangleLinesEx(ed->level->bgs[i].dest, 2, RED);
        }
    }
    for (int i = 0; i < ed->level->enemyCount; i++) {
        int ext = 25;
        if (is_state_enemy(ed->state) &&
            i == ed->selected) {
            DrawCircleLines(ed->level->enemies[i].position.x, ed->level->enemies[i].position.y, 25, RED);
            DrawCircleLines(ed->level->enemies[i].position.x, ed->level->enemies[i].position.y, 24, RED);
            DrawCircleLines(ed->level->enemies[i].position.x, ed->level->enemies[i].position.y, 23, RED);
        }
    }

    // draw win_area
    DrawRectangleLinesEx(ed->level->win_area, 2, GREEN);
    if (is_state_wa(ed->state)) {
        DrawRectangleLinesEx(ed->level->win_area, 2, RED);
    }

    EndMode2D();
}




void editor_create_new_wall(LevelEditor* ed) {
    if (ed->level->wallCount == MAX_WALLS) {
        fprintf(stderr, "editor_create_new_wall: MAX_WALLS count reached in level->wallCount");
        return;
    }

    Wall* w = &ed->level->walls[ed->level->wallCount++];
    Wall new_wall = {0};
    new_wall.rect.x = ed->mouse_world.x;
    new_wall.rect.y = ed->mouse_world.y;
    new_wall.rect.width = 25;
    new_wall.rect.height = 25;
    *w = new_wall;
}

void editor_create_new_enemy(LevelEditor* ed, EnemyType type) {
    if (ed->level->enemyCount == MAX_ENEMIES) {
        fprintf(stderr, "editor_create_new_enemy: MAX_ENEMIES count reached in level->enemyCount");
        return;
    }
    Entity* e = &ed->level->enemies[ed->level->enemyCount++];
    *e = InitEnemy(ed->mouse_world, type);
}

void editor_create_new_door(LevelEditor* ed) {
    if (ed->level->doorCount == MAX_DOORS) {
        fprintf(stderr, "editor_create_new_door: MAX_DOORS count reached in level->doorCount");
        return;
    }

    Door* w = &ed->level->doors[ed->level->doorCount++];
    Door new_door = {0};
    new_door.rect.x = ed->mouse_world.x;
    new_door.rect.y = ed->mouse_world.y;
    new_door.rect.width = 25;
    new_door.rect.height = 25;
    *w = new_door;
}


void editor_delete_entity(LevelEditor* ed) {
    if (ed->selected < 0) return;
    int idx = ed->selected;

    if (is_state_wall(ed->state)) {
        if (idx >= ed->level->wallCount) return;
        size_t num_to_move = ed->level->wallCount - idx - 1;
        if (num_to_move > 0) {
            memmove(&ed->level->walls[idx], &ed->level->walls[idx+1], num_to_move * sizeof(Wall));
        }
        ed->level->wallCount--;
        ed->selected = -1;
        ed->state = ED_IDLE;
    }
    else if (is_state_enemy(ed->state)) {
        if (idx >= ed->level->enemyCount) return;
        size_t num_to_move = ed->level->enemyCount - idx - 1;
        if (num_to_move > 0) {
            memmove(&ed->level->enemies[idx], &ed->level->enemies[idx+1], num_to_move * sizeof(Entity));
        }
        ed->level->enemyCount--;
        ed->selected = -1;
        ed->state = ED_IDLE;
    }
    else if (is_state_door(ed->state)) {
        if (idx >= ed->level->doorCount) return;
        size_t num_to_move = ed->level->doorCount - idx - 1;
        if (num_to_move > 0) {
            memmove(&ed->level->doors[idx], &ed->level->doors[idx+1], num_to_move * sizeof(Door));
        }
        ed->level->doorCount--;
        ed->selected = -1;
        ed->state = ED_IDLE;
    }
}

void LevelEditor_update(LevelEditor* ed, Camera2D* camera) {
    if (!ed || !ed->level) return;

    if (IsKeyPressed(KEY_O)) {
        ed->state = (ed->state == ED_CLOSED) ? ED_IDLE : ED_CLOSED;
        ed->selected = -1;
    }

    if (ed->state == ED_CLOSED)  return;

    if (IsKeyPressed(KEY_N)) {
        editor_create_new_wall(ed);
    }
    if (IsKeyPressed(KEY_L)) {
        editor_delete_entity(ed);
    }
    if (IsKeyPressed(KEY_ONE)) {
        editor_create_new_enemy(ed, ENEMY_CIVILIAN);
    } else if (IsKeyPressed(KEY_TWO)) {
        editor_create_new_enemy(ed, ENEMY_STAFF);
    } else if (IsKeyPressed(KEY_THREE)) {
        editor_create_new_enemy(ed, ENEMY_GUARD);
    } else if (IsKeyPressed(KEY_FOUR)) {
        editor_create_new_enemy(ed, ENEMY_ADMIN);
    }
    if (IsKeyPressed(KEY_B)) {
        editor_create_new_door(ed);
    }


    ed->mouse_world = GetScreenToWorld2D(GetMousePosition(), *camera);
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        editor_pick(ed);
    }
    editor_update_state(ed);
    editor_update(ed, camera);

    if (IsKeyPressed(KEY_I)) {
        level_editor_export(ed);
    }

    editor_draw_debug(ed, camera);
    DrawText(
        TextFormat(
            "--- EDITOR ---\n"
            "EditorState: %s\n"
            "Selected id: %d\n\n"
            "M: MOVE | S: SCALE |  R: ROTATE\n"
            "L: Lose (delete) entity\n"
            "N: New wall | B: new Door\n"
            "Create Enemy:  1: Civilian | 2: Staff | 3: Guard | 4: Admin\n"
            "I: export level",
            EditorState_cstr(ed->state),
            ed->selected
        ),
        10, 800, 20, YELLOW
    );
}


