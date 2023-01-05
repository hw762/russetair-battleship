#pragma once

#include <vulkan/vulkan.h>

struct PhysicalDevice;
typedef struct PhysicalDevice PhysicalDevice;
struct Queue;
typedef struct Queue Queue;

typedef struct Device {
    VkDevice handle;
    const PhysicalDevice* phys;
} Device;

Device newRenderDevice(PhysicalDevice* arrPhysicalDevices);

Queue deviceGetGraphicsQueue(const Device* device);
Queue deviceGetPresentQueue(const Device* device, VkSurfaceKHR surface);
