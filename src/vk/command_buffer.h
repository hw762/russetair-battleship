#pragma once

#include <vulkan/vulkan.h>

struct Device;
typedef struct Device Device;
struct Queue;
typedef struct Queue Queue;
struct ImageView;
typedef struct ImageView ImageView;

typedef struct CommandPool {
    VkCommandPool handle;
    const Device* device;
} CommandPool;

CommandPool newCommandPool(const Device* device, const Queue* queue);

typedef struct CommandBuffer {
    VkCommandBuffer handle;
    const CommandPool* commandPool;
} CommandBuffer;

CommandBuffer newCommandBuffer(const CommandPool* pool);
void commandBufferRecordClear(const CommandBuffer* commandBuffer,
    const ImageView* view, uint32_t w, uint32_t h);