#pragma once

#include <vulkan/vulkan.h>

struct PhysicalDevice;
typedef struct PhysicalDevice PhysicalDevice;

typedef struct RenderDevice {
    VkDevice handle;
    const PhysicalDevice* phys;
    VkQueue queue;
} RenderDevice;

RenderDevice newRenderDevice(PhysicalDevice* arrPhysicalDevices);