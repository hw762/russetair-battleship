#include "graphics.h"

#include <SDL.h>
#include <SDL_vulkan.h>
#include <limits.h>
#include <stb_ds.h>

#include "renderer/clear_screen.h"
#include "renderer/renderer.h"
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
    VulkanSystem system = newVulkanSystem(extensions, n_extensions);
    free(extensions);

    VkSurfaceKHR surface;
    if (!SDL_Vulkan_CreateSurface(*pWindow, system.instance.vkInstance, &surface)) {
        ecs_abort(1, "Failed to create Vulkan surface: %s", SDL_GetError());
    }
    Queue presentQueue = deviceGetPresentQueue(&system.renderDevice, surface);
    int w, h;
    SDL_Vulkan_GetDrawableSize(*pWindow, &w, &h);
    Swapchain swapchain;
    SwapchainCreateInfo swapchainCI = {
        .pDevice = &system.renderDevice,
        .surface = surface,
        .requestedImages = 3,
        .vSync = true,
        .defaultWidth = w,
        .defaultHeight = w,
    };
    createSwapchain(&swapchainCI, &swapchain);

    // Render passes
    RenderPass rpass;
    RenderPassCreateInfo rpassCreateInfo = {
        .format = swapchain.format.imageFormat,
        .pDevice = &system.renderDevice,
    };
    createDefaultRenderPass(&rpassCreateInfo, &rpass);

    Framebuffer* arrFramebuffer = NULL;
    arrsetlen(arrFramebuffer, arrlen(swapchain.arrViews));

    CommandPool pool = newCommandPool(&system.renderDevice, &presentQueue);

    ClearScreenRenderer renderer;
    ClearScreenRendererCreateInfo rendererCI = {
        .clearColor = { .float32 = { 0.1, 0.2, 0.3, 1.0 } },
    };
    createClearScreenRenderer(&rendererCI, &renderer);

    // Pre-record clears
    for (int i = 0; i < arrlen(swapchain.arrViews); ++i) {
        FramebufferCreateInfo fbCI = {
            .pImageView = &swapchain.arrViews[i],
            .pRenderPass = &rpass,
            .width = swapchain.extent.width,
            .height = swapchain.extent.height,
        };
        createFramebuffer(&fbCI, &arrFramebuffer[i]);
        CommandBuffer cmdBuf = newCommandBuffer(&pool);
        RendererRecordInfo recInfo = {
            .commandBuffer = cmdBuf.handle,
            .framebuffer = arrFramebuffer[i].vkFramebuffer,
            .renderPass = rpass.vkRenderPass,
            .extent = { .width = w, .height = h },
        };
        clearScreenRendererRecord(renderer, &recInfo);
        // Render to screen
        swapchainAcquire(&swapchain);
        // TODO: queue submit
        const ImageView* view = swapchainCurrentView(&swapchain);
        vkWaitForFences(system.renderDevice.vkDevice, 1, &view->fence, true, LONG_MAX);
        vkResetFences(system.renderDevice.vkDevice, 1, &view->fence);
        queueSubmit(&presentQueue, &cmdBuf,
            view->acquisitionSemaphore, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            view->renderCompleteSemaphore, view->fence);

        swapchainPresent(&swapchain, presentQueue.handle);
        // vkFreeCommandBuffers(instance.renderDevice.handle, pool.handle, 1, &cmdBuf.handle);
    }

    return e;
}