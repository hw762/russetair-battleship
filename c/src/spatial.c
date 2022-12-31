#include "spatial.h"

ECS_COMPONENT_DECLARE(Position);
ECS_COMPONENT_DECLARE(Rotation);
ECS_COMPONENT_DECLARE(Velocity);
ECS_COMPONENT_DECLARE(AngularVelocity);
ECS_COMPONENT_DECLARE(Acceleration);

void spatial_register(ecs_world_t* ecs)
{
    ECS_COMPONENT_DEFINE(ecs, Position);
    ECS_COMPONENT_DEFINE(ecs, Rotation);
    ECS_COMPONENT_DEFINE(ecs, Velocity);
    ECS_COMPONENT_DEFINE(ecs, AngularVelocity);
    ECS_COMPONENT_DEFINE(ecs, Acceleration);
}
