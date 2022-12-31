#pragma once

#include <vulkan/vulkan.h>

struct RenderDevice;
typedef struct RenderDevice RenderDevice;

struct ImageView;
typedef struct ImageView ImageView;

typedef struct Pipeline {
    VkPipeline handle;
} Pipeline;

// Pipeline
// newPipelineClearSwapchain(const Swapchain* swapchain);

typedef struct CommandPool {
    VkCommandPool handle;
    const RenderDevice* device;
} CommandPool;

CommandPool newCommandPool(const RenderDevice* device);

typedef struct CommandBuffer {
    VkCommandBuffer handle;
    const CommandPool* commandPool;
} CommandBuffer;

CommandBuffer newCommandBuffer(const CommandPool* pool);
void commandBufferRecordClear(const CommandBuffer* commandBuffer,
    const ImageView* view, uint32_t w, uint32_t h);