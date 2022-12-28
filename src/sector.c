#include "sector.h"

ECS_COMPONENT_DECLARE(SectorCoord);
ECS_COMPONENT_DECLARE(SectorHeight);

void registerSector(ecs_world_t* ecs)
{
    ECS_COMPONENT_DEFINE(ecs, SectorCoord);
    ECS_COMPONENT_DEFINE(ecs, SectorHeight);
}

ecs_entity_t spawnSector(ecs_world_t* ecs, int x, int y, float h, ecs_entity_t (*chunk_spawner)(ecs_world_t*, int, int, float))
{
    ecs_trace("Spawning sector [%d, %d, %f]", x, y, h);
    ecs_entity_t e = ecs_new_id(ecs);
    ecs_set(ecs, e, SectorCoord, { .x = x, .y = y });
    ecs_set(ecs, e, SectorHeight, { .h = h });
    if (chunk_spawner == NULL) {
        return e;
    }
    for (int i = 0; i < SECTOR_SIZE; ++i) {
        for (int j = 0; j < SECTOR_SIZE; ++j) {
            ecs_entity_t c = chunk_spawner(ecs, x, y, h);
            ecs_add_pair(ecs, c, EcsChildOf, e);
        }
    }
    return e;
}
