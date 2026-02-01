#include "levels.h"

// Access to episodes
void InitEpisode1(Level *level); // Prototype from episodes/episode1.c (usually in a header)
// Actually main.c included "episodes/episodes.h". Use that.
#include "episodes/episodes.h"

void InitLevel(int episode, Level *level) {
  // Unload previous episode's assets first
  static int lastEpisode = -1;
  if (lastEpisode >= 0 && lastEpisode != episode) {
    UnloadLevel(lastEpisode);
  }
  lastEpisode = episode;

  // Reset entire level struct
  *level = (Level){0};
  level->id = episode;

  switch (episode) {
  case 0:
    InitProlog(level);
    break;
  case 1:
    InitEpisode1(level);
    break;
  case 2:
    InitEpisode2(level);
    break;
  case 3:
    InitEpisode3(level);
    break;
  case 4:
    InitEpisode4(level);
    break;
  default:
    TraceLog(LOG_WARNING, "Episode %d not found!", episode);
    break;
  }
}

void UnloadLevel(int episode) {
  switch (episode) {
  case 0: UnloadProlog(); break;
  case 1: UnloadEpisode1(); break;
  case 2: UnloadEpisode2(); break;
  case 3: UnloadEpisode3(); break;
  case 4: UnloadEpisode4(); break;
  }
}
