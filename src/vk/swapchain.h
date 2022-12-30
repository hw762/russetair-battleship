#pragma once

#include <flecs.h>

ecs_entity_t createSwapchain(ecs_world_t* ecs,
    ecs_entity_t eSystem,
    int requestedImages, bool vSync);