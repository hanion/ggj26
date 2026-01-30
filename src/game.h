#ifndef GAME_H
#define GAME_H

#include <stdbool.h>

void Game_Init(void);
void Game_Update(void);
void Game_Draw(void);
void Game_Shutdown(void); // Optional, for cleanup

// Global or shared constants if needed
#define INITIAL_SCREEN_WIDTH 1280
#define INITIAL_SCREEN_HEIGHT 720

#endif // GAME_H
