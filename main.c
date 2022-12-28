const char* PROJECT_NAME = MESON_PROJECT_NAME;
const char* ENGINE_NAME = "PotatoEngine";

#include <flecs.h>
#include <stdio.h>
#include <stdlib.h>

#include "chunk.h"
#include "graphics.h"
#include "sector.h"

typedef struct {
    ecs_world_t* ecs;
} Game;

void gameInit(Game* game)
{
    game->ecs = ecs_init();
    registerGraphics(game->ecs);
    registerSector(game->ecs);
    registerChunk(game->ecs);
}

void cleanupGame(Game* game)
{
    ecs_fini(game->ecs);
}

int main()
{
    Game game;
    gameInit(&game);

    ecs_log_set_level(0);

    createGraphicsSystem(game.ecs);
    // spawnSector(game.ecs, 0, 0, 0, &spawnChunkDefault);
    // spawnSector(game.ecs, 0, 1, 0, &spawnChunk);
    // spawnSector(game.ecs, 1, 0, 0, NULL);
    // spawnSector(game.ecs, 1, 1, 0, NULL);

    cleanupGame(&game);
}
