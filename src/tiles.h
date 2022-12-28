#pragma once

#include <flecs.h>

typedef struct {
    int x, y;
} TilePosition;

typedef struct {
    float h;
} TileHeight;

void tile_register(ecs_world_t* ecs);

ecs_entity_t tile_spawn(ecs_world_t* ecs,
    int x, int y, float h);

/// @brief Mapping of chunk to tile. SxS tiles per chunk.
#define CHUNK_SIZE 16

/// @brief Chunk coordinates, which is tile coordinates divided by 16
typedef struct {
    int x, y;
} ChunkPosition;

typedef struct {
    float h;
} ChunkHeight;

void chunk_register(ecs_world_t* ecs);
ecs_entity_t chunk_spawn(ecs_world_t* ecs, int x, int y, float h);