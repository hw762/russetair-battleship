#include "buffer.h"

#include <flecs.h>
#include <stdlib.h>
#include <vk_mem_alloc.h>

#include "check.h"
#include "device.h"
#include "memory.h"

void createBuffer(const Device* pDevice, VkDeviceSize size,
    VkBufferUsageFlags bufferUsage, VmaMemoryUsage memoryUsage,
    VkMemoryPropertyFlags requiredFlags, Buffer* pBuffer)
{
    VkBufferCreateInfo info = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = size,
        .usage = bufferUsage,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
    };
    VmaAllocationCreateInfo allocInfo = {
        .requiredFlags = requiredFlags,
        .usage = memoryUsage,
    };
    VkBuffer buf;
    VmaAllocation alloc;
    vkCheck(vmaCreateBuffer(pDevice->allocator.vmaAllocator, &info, &allocInfo, &buf, &alloc, NULL))
    {
        ecs_abort(1, "Failed to create buffer");
    }
    *pBuffer = (Buffer) {
        .vkBuffer = buf,
        .vmaAllocation = alloc,
        .pDevice = pDevice,
    };
}

void destroyBuffer(Buffer* pBuffer)
{
    bufferUnmap(pBuffer);
    vmaDestroyBuffer(pBuffer->pDevice->allocator.vmaAllocator, pBuffer->vkBuffer, pBuffer->vmaAllocation);
}

void* bufferMap(Buffer* pBuffer)
{
    if (pBuffer->pMappedMemory == NULL) {
        vkCheck(vmaMapMemory(pBuffer->pDevice->allocator.vmaAllocator, pBuffer->vmaAllocation, &pBuffer->pMappedMemory))
        {
            ecs_abort(1, "Failed to map buffer");
        }
    }
    return pBuffer->pMappedMemory;
}

void bufferUnmap(Buffer* pBuffer)
{
    if (pBuffer->pMappedMemory != NULL) {
        vmaUnmapMemory(pBuffer->pDevice->allocator.vmaAllocator, pBuffer->vmaAllocation);
        pBuffer->pMappedMemory = NULL;
    }
}