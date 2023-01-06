#pragma once

#include <vk_mem_alloc.h>

typedef struct Device Device;
typedef struct VulkanInstance VulkanInstance;

typedef struct MemoryAllocator {
    VmaAllocator vmaAllocator;
} MemoryAllocator;

void newMemoryAllocator(const VulkanInstance* pInstance, const Device* pDev, MemoryAllocator* pAllocator);