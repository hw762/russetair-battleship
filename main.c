const char* PROJECT_NAME = MESON_PROJECT_NAME;
const char* ENGINE_NAME = "PotatoEngine";

#include <flecs.h>
#include <stdio.h>

#include "tiles.h"

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
    game_init(&game);

    tile_register(game.ecs);
    chunk_register(game.ecs);
    chunk_spawn(game.ecs, 0, 0, 0.0);

    game_destroy(&game);
}
