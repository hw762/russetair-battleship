#pragma once

#include <flecs.h>

extern ECS_COMPONENT_DECLARE(GraphicsSystem);

extern ECS_COMPONENT_DECLARE(SDLWindowPtr);

void registerGraphics(ecs_world_t* ecs);

ecs_entity_t createGraphicsSystem(ecs_world_t* ecs);
