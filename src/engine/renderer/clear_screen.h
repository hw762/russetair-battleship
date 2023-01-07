#pragma once

#include <vulkan/vulkan.h>

typedef struct Device Device;
typedef struct Swapchain Swapchain;
typedef struct CommandPool CommandPool;
typedef struct CommandBuffer CommandBuffer;
typedef struct Framebuffer Framebuffer;
typedef struct ImageView ImageView;
typedef struct PipelineCache PipelineCache;

typedef struct ClearScreenRenderer {
    VkPipeline vkPipeline;
} ClearScreenRenderer;

typedef struct ClearScreenRendererCreateInfo {
    const Device* pDevice;
    const VkClearColorValue clearColor;
} ClearScreenRendererCreateInfo;

/// TODO: put this inside renderer
void commandBufferRecordClear(const CommandBuffer* commandBuffer,
    const Framebuffer* frameBuffer,
    const ImageView* view, uint32_t w, uint32_t h);