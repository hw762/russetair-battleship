#pragma once

#include <vk_mem_alloc.h>
#include <vulkan/vulkan.h>

typedef struct Device Device;

typedef struct Buffer {
    VkBuffer vkBuffer;
    VmaAllocation vmaAllocation;
    void* pMappedMemory;
    const Device* pDevice;
} Buffer;

void createBuffer(const Device* pDevice, VkDeviceSize size,
    VkBufferUsageFlags bufferUsage, VmaMemoryUsage memoryUsage,
    VkMemoryPropertyFlags requiredFlags, Buffer* pBuffer);
void destroyBuffer(Buffer* pBuffer);

void* bufferMap(Buffer* pBuffer);
void bufferUnmap(Buffer* pBuffer);
