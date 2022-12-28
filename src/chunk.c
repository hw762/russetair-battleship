#include "chunk.h"

ECS_COMPONENT_DECLARE(ChunkCoord);
ECS_COMPONENT_DECLARE(ChunkHeight);
ECS_COMPONENT_DECLARE(TileHeights);

void registerChunk(ecs_world_t* ecs)
{
    ECS_COMPONENT_DEFINE(ecs, ChunkCoord);
    ECS_COMPONENT_DEFINE(ecs, ChunkHeight);
    ECS_COMPONENT_DEFINE(ecs, TileHeights);
}

ecs_entity_t spawnChunkDefault(ecs_world_t* ecs, int x, int y, float h)
{
    ecs_entity_t e = spawnChunk(ecs, x, y, h);
    TileHeights* tiles = ecs_emplace(ecs, e, TileHeights);
    for (int i = 0; i < CHUNK_SIZE; ++i) {
        for (int j = 0; j < CHUNK_SIZE; ++j) {
            tiles->heights[i * CHUNK_SIZE + j] = h;
        }
    }
    return e;
}

ecs_entity_t spawnChunk(ecs_world_t* ecs, int x, int y, float h)
{
    ecs_entity_t e = ecs_new_id(ecs);
    ecs_set(ecs, e, ChunkCoord, { .x = x, .y = y });
    ecs_set(ecs, e, ChunkHeight, { .h = h });
    return e;
}
