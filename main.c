const char* PROJECT_NAME = MESON_PROJECT_NAME;
const char* ENGINE_NAME = "PotatoEngine";

#include <flecs.h>
#include <stdio.h>

#include "chunk.h"
#include "sector.h"

struct game {
    ecs_world_t* ecs;
};

void game_init(struct game* game)
{
    game->ecs = ecs_init();
}

void game_destroy(struct game* game)
{
    ecs_fini(game->ecs);
}

int main()
{
    struct game game;
    // ecs_log_set_level(3);
    ecs_trace("Russetair Battleship.");
    game_init(&game);

    sector_register(game.ecs);
    chunk_register(game.ecs);

    sector_spawn(game.ecs, 0, 0, 0, &chunk_spawn_default);
    sector_spawn(game.ecs, 0, 1, 0, &chunk_spawn);
    sector_spawn(game.ecs, 1, 0, 0, NULL);
    sector_spawn(game.ecs, 1, 1, 0, NULL);

    game_destroy(&game);
}
