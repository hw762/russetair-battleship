#include "tiles.h"

ECS_COMPONENT_DECLARE(TilePosition);
ECS_COMPONENT_DECLARE(TileHeight);
ECS_COMPONENT_DECLARE(ChunkPosition);
ECS_COMPONENT_DECLARE(ChunkHeight);

void tile_register(ecs_world_t* ecs)
{
    ECS_COMPONENT_DEFINE(ecs, TilePosition);
    ECS_COMPONENT_DEFINE(ecs, TileHeight);
}

ecs_entity_t tile_spawn(ecs_world_t* ecs, int x, int y, float h)
{
    ecs_entity_t e = ecs_new_id(ecs);
    ecs_set(ecs, e, TilePosition, { .x = x, .y = y });
    ecs_set(ecs, e, TileHeight, { .h = h });
    return e;
}

void chunk_register(ecs_world_t* ecs)
{
    ECS_COMPONENT_DEFINE(ecs, ChunkPosition);
    ECS_COMPONENT_DEFINE(ecs, ChunkHeight);
}

ecs_entity_t chunk_spawn(ecs_world_t* ecs, int x, int y, float h)
{
    ecs_entity_t e = ecs_new_id(ecs);
    ecs_set(ecs, e, ChunkPosition, { .x = x, .y = y });
    ecs_set(ecs, e, ChunkHeight, { .h = h });
    for (int i = 0; i < CHUNK_SIZE; ++i) {
        for (int j = 0; j < CHUNK_SIZE; ++j) {
            ecs_entity_t t = tile_spawn(ecs, x * 16 + i, y * 16 + j, h);
            ecs_add_pair(ecs, t, EcsChildOf, e);
        }
    }
    return e;
}
