#pragma once

#include <vk_mem_alloc.h>

typedef struct Device Device;
typedef struct VulkanSystem VulkanSystem;

typedef struct MemoryAllocator {
    VmaAllocator vmaAllocator;
} MemoryAllocator;

void createMemoryAllocator(const Device* pDev, MemoryAllocator* pAllocator);