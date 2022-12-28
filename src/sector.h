#pragma once

#include <flecs.h>

/// @brief Number of chunks per edge of a sector
#define SECTOR_SIZE 64
#define SECTOR_AREA (SECTOR_SIZE * SECTOR_SIZE)

/// @brief Sector coordinates, which is chunk coordinates / 16
typedef struct {
    int x, y;
} SectorCoord;

/// @brief Nominal height of a sector, used for sector-level computation
typedef struct {
    float h;
} SectorHeight;

void sector_register(ecs_world_t* ecs);

/// @brief Spawn a sector
/// @param ecs
/// @param x Sector coordinate X
/// @param y Sector coordinate Y
/// @param h Sector height
/// @param chunk_spawner Chunk spawn function. If NULL, chunks are not spawned
/// @return Sector ID
ecs_entity_t sector_spawn(ecs_world_t* ecs, int x, int y, float h,
    ecs_entity_t (*chunk_spawner)(ecs_world_t*, int, int, float));