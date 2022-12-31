#pragma once

#include <stdbool.h>
#include <vulkan/vulkan.h>

#include "device.h"

typedef struct SurfaceFormat {
    int imageFormat;
    int colorSpace;
} SurfaceFormat;

typedef struct ImageView {
    VkImageView handle;
    VkFence fence;
    VkSemaphore acquisitionSemaphore;
    VkSemaphore renderCompleteSemaphore;
} ImageView;

typedef struct Swapchain {
    VkSwapchainKHR handle;
    const RenderDevice* device;
    VkSurfaceKHR surface;
    ImageView* arrViews;
    SurfaceFormat format;

    uint32_t currentFrame;
} Swapchain;

Swapchain
newSwapchain(const RenderDevice* renderDevice, VkSurfaceKHR surface,
    int requestedImages, bool vsync, uint32_t defaultWidth, uint32_t defaultHeight);

bool swapchainAcquire(Swapchain* swapchain);
const ImageView* swapchainCurrentView(const Swapchain* swapchain);
bool swapchainPresent(Swapchain* swapchain, VkQueue queue);