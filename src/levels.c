#include "levels.h"

// Access to episodes
void InitEpisode1(Level *level); // Prototype from episodes/episode1.c (usually in a header)
// Actually main.c included "episodes/episodes.h". Use that.
#include "episodes/episodes.h"

void InitLevel(int episode, Level *level) {
  level->id = episode;
  level->wallCount = 0;
  level->doorCount = 0;
  level->enemyCount = 0;

  switch (episode) {
  case 1:
    InitEpisode1(level);
    break;
  case 2:
    InitEpisode2(level);
    break;
  default:
    TraceLog(LOG_WARNING, "Episode %d not found!", episode);
    break;
  }
}

void UnloadLevel(int episode) {
  switch (episode) {
  case 1: UnloadEpisode1(); break;
  case 2: UnloadEpisode2(); break;
  }
}
