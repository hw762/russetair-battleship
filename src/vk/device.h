#pragma once

#include <vulkan/vulkan.h>

#include "memory.h"

struct PhysicalDevice;
typedef struct PhysicalDevice PhysicalDevice;
struct Queue;
typedef struct Queue Queue;

typedef struct Device {
    VkDevice vkDevice;
    MemoryAllocator allocator;
    //
    const PhysicalDevice* phys;
} Device;

void createDevice(const PhysicalDevice* arrPhysicalDevices, Device* pDevice);
void destroyDevice(Device* pDevice);

Queue deviceGetGraphicsQueue(const Device* device);
Queue deviceGetPresentQueue(const Device* device, VkSurfaceKHR surface);
