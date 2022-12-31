#pragma once

#include <vulkan/vulkan.h>

struct RenderDevice;
typedef struct RenderDevice RenderDevice;
struct Swapchain;
typedef struct Swapchain Swapchain;

typedef struct RenderPass {
    VkRenderPass handle;
    const RenderDevice* device;
} RenderPass;

RenderPass
newRenderPassClearSwapchain(const RenderDevice* device, const Swapchain* swapchain);