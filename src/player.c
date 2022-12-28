#include "player.h"

extern ECS_COMPONENT_DECLARE(Position);
extern ECS_COMPONENT_DECLARE(Rotation);

ECS_COMPONENT_DECLARE(PlayerControlled);
ECS_COMPONENT_DECLARE(Spectator);

void player_register(ecs_world_t* ecs)
{
    ECS_TAG(ecs, PlayerControlled);
    ECS_TAG(ecs, Spectator);
}

void spectator_spawn(ecs_world_t* ecs, Position pos, Rotation rot)
{
    ecs_entity_t e = ecs_new_id(ecs);

    ecs_add(ecs, e, PlayerControlled);
    ecs_add(ecs, e, Spectator);
    ecs_set_ptr(ecs, e, Position, &pos);
    ecs_set_ptr(ecs, e, Rotation, &rot);
}
