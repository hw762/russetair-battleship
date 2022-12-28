#pragma once

#include <flecs.h>

extern ECS_COMPONENT_DECLARE(ChunkCoord);
extern ECS_COMPONENT_DECLARE(ChunkHeight);
extern ECS_COMPONENT_DECLARE(TileHeights);

/// @brief Number of tiles per edge of a chunk
#define CHUNK_SIZE 16
#define CHUNK_AREA (CHUNK_SIZE * CHUNK_SIZE)

/// @brief Chunk coordinates, which is tile coordinates divided by 16
typedef struct {
    int x, y;
} ChunkCoord;

/// @brief Nominal height of a chunk, used for chunk-level computation
typedef struct {
    float h;
} ChunkHeight;

/// @brief All heights in a chunk
typedef struct {
    float heights[CHUNK_AREA];
} TileHeights;

void chunk_register(ecs_world_t* ecs);

/// @brief Spawn a new chunk with all its tiles set to default height
/// @param ecs
/// @param x Chunk coordinate X
/// @param y Chunk coordinate Y
/// @param h Chunk height
/// @return Chunk ID
ecs_entity_t chunk_spawn_default(ecs_world_t* ecs, int x, int y, float h);

/// @brief Spawn a new chunk without its tiles
/// @param ecs
/// @param x
/// @param y
/// @param h
/// @return Chunk ID
ecs_entity_t chunk_spawn(ecs_world_t* ecs, int x, int y, float h);