#pragma once

#include <vulkan/vulkan.h>

struct Device;
typedef struct Device Device;
struct CommandBuffer;
typedef struct CommandBuffer CommandBuffer;

typedef struct Queue {
    VkQueue handle;
    const Device* device;
    int queueFamilyIndex;
} Queue;

void queueSubmit(const Queue* queue, const CommandBuffer* cmdBuf,
    VkSemaphore waitSemaphore, VkPipelineStageFlags dstStageMask,
    VkSemaphore signalSemaphore, VkFence fence);