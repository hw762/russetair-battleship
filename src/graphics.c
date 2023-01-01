#include "graphics.h"

#include <SDL.h>
#include <SDL_vulkan.h>
#include <limits.h>
#include <stb_ds.h>

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

    SDLWindowPtr* pWindow = ecs_emplace(ecs, e, SDLWindowPtr);
    uint32_t n_extensions;
    const char** extensions = NULL;
    *pWindow = SDL_CreateWindow(
        PROJECT_NAME, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        1280, 720, SDL_WINDOW_VULKAN | SDL_WINDOW_ALLOW_HIGHDPI);
    if (!*pWindow) {
        ecs_abort(1, "SDL init failed: %s", SDL_GetError());
    }

    if (!SDL_Vulkan_GetInstanceExtensions(*pWindow, &n_extensions, NULL)) {
        ecs_abort(1, "Failed to get number of required extensions: %s", SDL_GetError());
    }
    extensions = malloc(sizeof(const char*) * n_extensions);
    if (!SDL_Vulkan_GetInstanceExtensions(*pWindow, &n_extensions, extensions)) {
        ecs_abort(1, "Failed to get required extensions: %s", SDL_GetError());
    }
    // ecs_entity_t system = ecs_new_w_pair(ecs, EcsIsA, VulkanSystem);
    VulkanInstance instance = newVulkanInstance(extensions, n_extensions);
    free(extensions);

    VkSurfaceKHR surface;
    if (!SDL_Vulkan_CreateSurface(*pWindow, instance.instance, &surface)) {
        ecs_abort(1, "Failed to create Vulkan surface: %s", SDL_GetError());
    }
    Queue presentQueue = deviceGetPresentQueue(&instance.renderDevice, surface);
    int w, h;
    SDL_Vulkan_GetDrawableSize(*pWindow, &w, &h);
    Swapchain swapchain
        = newSwapchain(&instance.renderDevice, surface, 3, true, w, h);
    CommandPool pool = newCommandPool(&instance.renderDevice, &presentQueue);
    CommandBuffer cmdBuf = newCommandBuffer(&pool);

    // Pre-record clears
    for (int i = 0; i < arrlen(swapchain.arrViews); ++i) {
        commandBufferRecordClear(&cmdBuf, &swapchain.arrViews[i], w, h);
    }
    // Render to screen
    swapchainAcquire(&swapchain);
    // TODO: queue submit
    const ImageView* view = swapchainCurrentView(&swapchain);
    queueSubmit(&presentQueue, &cmdBuf,
        view->acquisitionSemaphore, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        view->renderCompleteSemaphore, view->fence);

    swapchainPresent(&swapchain, presentQueue.handle);

    return e;
}
