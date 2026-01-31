#include "../raylib/src/raylib.h"
#include "game.h"

int main(void) {
#if defined(__APPLE__)
  SetConfigFlags(FLAG_WINDOW_HIGHDPI);
#endif
  InitWindow(0, 0,
             "Identity Theft - Modular");
  SetTargetFPS(60);
  SetWindowState(FLAG_WINDOW_RESIZABLE);

  Game_Init();

  while (!WindowShouldClose()) {
    Game_Update();
    Game_Draw();
  }

  Game_Shutdown();
  CloseWindow();
  return 0;
}
