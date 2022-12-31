#pragma once

#include <vulkan/vulkan.h>

#include "device.h"

typedef struct SurfaceFormat {
    int imageFormat;
    int colorSpace;
} SurfaceFormat;

typedef struct Swapchain {
    VkSwapchainKHR handle;
    const RenderDevice* device;
    VkSurfaceKHR surface;
    VkImageView* arrViews;
    SurfaceFormat format;
} Swapchain;