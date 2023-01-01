#pragma once

#include <vulkan/vulkan.h>

struct PhysicalDevice;
typedef struct PhysicalDevice PhysicalDevice;

struct CommandBuffer;
typedef struct CommandBuffer CommandBuffer;

typedef struct Device {
    VkDevice handle;
    const PhysicalDevice* phys;
} Device;

Device newRenderDevice(PhysicalDevice* arrPhysicalDevices);

typedef struct Queue {
    VkQueue handle;
    const Device* device;
    int queueFamilyIndex;
} Queue;
Queue deviceGetGraphicsQueue(const Device* device);
Queue deviceGetPresentQueue(const Device* device, VkSurfaceKHR surface);
void queueSubmit(const Queue* queue, const CommandBuffer* cmdBuf, VkSemaphore waitSemaphore, int dstStageMask, VkSemaphore signalSemaphore, VkFence fence);