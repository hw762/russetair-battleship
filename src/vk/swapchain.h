#pragma once

#include <flecs.h>

struct SurfaceFormat {
    int imageFormat;
    int colorSpace;
};

void registerSwapchain(ecs_world_t* ecs);