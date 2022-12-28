#include "graphics.h"

#include <SDL.h>
#include <SDL_vulkan.h>

#include "vk/vk.h"

extern const char* PROJECT_NAME;

typedef SDL_Window* SDLWindowPtr;

ECS_DECLARE(GraphicsSystem);
ECS_COMPONENT_DECLARE(SDLWindowPtr);

void registerGraphics(ecs_world_t* ecs)
{
    ECS_TAG_DEFINE(ecs, GraphicsSystem);
    ECS_COMPONENT_DEFINE(ecs, SDLWindowPtr);
    registerVulkan(ecs);
}

ecs_entity_t createGraphicsSystem(ecs_world_t* ecs)
{
    SDL_Init(SDL_INIT_EVERYTHING);
    ecs_entity_t e = ecs_new_id(ecs);

    ecs_add(ecs, e, GraphicsSystem);

    SDLWindowPtr* window_p = ecs_emplace(ecs, e, SDLWindowPtr);
    uint32_t n_extensions;
    const char** extensions = NULL;
    *window_p = SDL_CreateWindow(
        PROJECT_NAME, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        1280, 720, SDL_WINDOW_VULKAN | SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_HIDDEN /* FIXME Hide until we can draw something */);
    if (!*window_p) {
        ecs_fatal("SDL init failed: %s", SDL_GetError());
        exit(1);
    }

    if (!SDL_Vulkan_GetInstanceExtensions(*window_p, &n_extensions, NULL)) {
        ecs_fatal("Failed to get number of required extensions: %s", SDL_GetError());
        exit(1);
    }
    extensions = malloc(sizeof(const char*) * n_extensions);
    if (!SDL_Vulkan_GetInstanceExtensions(*window_p, &n_extensions, extensions)) {
        ecs_fatal("Failed to get required extensions: %s", SDL_GetError());
        exit(1);
    }
    ecs_entity_t instance = createVulkanInstance(ecs, extensions, n_extensions);
    ecs_add_pair(ecs, instance, EcsChildOf, e);

    free(extensions);
    return e;
}