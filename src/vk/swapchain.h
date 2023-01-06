#pragma once

#include <stdbool.h>
#include <vulkan/vulkan.h>

#include "device.h"

typedef struct SurfaceFormat {
    int imageFormat;
    int colorSpace;
} SurfaceFormat;

typedef struct ImageView ImageView;

typedef struct Swapchain {
    VkSwapchainKHR handle;
    const Device* device;
    VkSurfaceKHR surface;
    ImageView* arrViews;
    SurfaceFormat format;
    VkExtent2D extent;

    uint32_t currentFrame;
} Swapchain;

typedef struct SwapchainCreateInfo {
    const Device* pDevice;
    VkSurfaceKHR surface;
    int requestedImages;
    bool vSync;
    uint32_t defaultWidth;
    uint32_t defaultHeight;
} SwapchainCreateInfo;

void createSwapchain(const SwapchainCreateInfo* pCreateInfo, Swapchain* pSwapchain);
void destroySwapchain(Swapchain* pSwapchain);

bool swapchainAcquire(Swapchain* swapchain);
const ImageView* swapchainCurrentView(const Swapchain* swapchain);
bool swapchainPresent(Swapchain* swapchain, VkQueue queue);