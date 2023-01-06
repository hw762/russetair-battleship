#pragma once

#include <vulkan/vulkan.h>

#include "memory.h"

struct PhysicalDevice;
typedef struct PhysicalDevice PhysicalDevice;
struct Queue;
typedef struct Queue Queue;

typedef struct Device {
    VkDevice handle;
    //
    const PhysicalDevice* phys;
} Device;

void createRenderDevice(PhysicalDevice* arrPhysicalDevices, Device* pDevice);

Queue deviceGetGraphicsQueue(const Device* device);
Queue deviceGetPresentQueue(const Device* device, VkSurfaceKHR surface);
