#pragma once

#include <vulkan/vulkan.h>

struct PhysicalDevice;
typedef struct PhysicalDevice PhysicalDevice;

typedef struct Queue {
    VkQueue handle;
    int queueFamilyIndex;
} Queue;

struct CommandBuffer;
typedef struct CommandBuffer CommandBuffer;

typedef struct RenderDevice {
    VkDevice handle;
    const PhysicalDevice* phys;
    Queue presentQueue;
} RenderDevice;

void queueSubmit(const RenderDevice* device, const CommandBuffer* cmdBuf, VkSemaphore waitSemaphore, int dstStageMask, VkSemaphore signalSemaphore, VkFence fence);

RenderDevice newRenderDevice(PhysicalDevice* arrPhysicalDevices);