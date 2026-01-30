#include "../raylib/src/raylib.h"
#include "game.h"

int main(void) {
  InitWindow(INITIAL_SCREEN_WIDTH, INITIAL_SCREEN_HEIGHT,
             "Identity Theft - Modular");
  SetTargetFPS(60);
  SetWindowState(FLAG_WINDOW_RESIZABLE);

  Game_Init();

  while (!WindowShouldClose()) {
    Game_Update();
    Game_Draw();
  }

  CloseWindow();
  return 0;
}
