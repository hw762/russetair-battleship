#pragma once

#include <vulkan/vulkan.h>

typedef struct Device Device;
typedef struct Swapchchain Swapchain;
typedef struct CommandPool CommandPool;
typedef struct PipelineCache PipelineCache;

typedef struct ClearScreenRenderer {
    VkPipeline vkPipeline;
} ClearScreenRenderer;

typedef struct ClearScreenRendererCreateInfo {
    const Device* pDevice;
    const VkClearColorValue clearColor;
} ClearScreenRendererCreateInfo;