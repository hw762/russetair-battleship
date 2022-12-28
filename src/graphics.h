#pragma once

#include <flecs.h>

extern ECS_COMPONENT_DECLARE(GraphicsSystem);

void graphics_register(ecs_world_t* ecs);

ecs_entity_t graphics_system_create(ecs_world_t* ecs);
