#ifndef GAME_H
#define GAME_H

#include <stdbool.h>

void Game_Init(void);
bool Game_Update(void);
void Game_Draw(void);
void Game_Shutdown(void); // Optional, for cleanup

#endif // GAME_H
