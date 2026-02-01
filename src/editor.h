#include "../raylib/src/raylib.h"
#include "levels.h"

typedef enum {
    ED_CLOSED = 0,

    ED_IDLE,

    ED_MOVE_WALL,
    ED_SCALE_WALL,
    ED_ROTATE_WALL,

    ED_MOVE_BG,
    ED_SCALE_BG,

    ED_MOVE_DOOR,
    ED_SCALE_DOOR,

    ED_MOVE_ENEMY,

    ED_MOVE_WA,
    ED_SCALE_WA,
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

LevelEditor LevelEditor_new(Level* level);
void LevelEditor_update(LevelEditor* ed, Camera2D* camera);

