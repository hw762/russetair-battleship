#include "memory.h"

#include <flecs.h>
#include <stdlib.h>
#include <vk_mem_alloc.h>
#include <vulkan/vulkan.h>

#include "check.h"
#include "device.h"
#include "instance.h"
#include "physical_device.h"
#include "vk.h"

void createMemoryAllocator(const Device* pDev, MemoryAllocator* pAllocator)
{
    VmaAllocatorCreateInfo ci = {
        .instance = pDev->phys->pInstance->vkInstance,
        .physicalDevice = pDev->phys->vkPhysicalDevice,
        .device = pDev->vkDevice,
    };
    vkIfFailed(vmaCreateAllocator(&ci, &pAllocator->vmaAllocator))
    {
        ecs_abort(1, "Failed to create VMA allocator");
    }
}

void destroyMemoryAllocator(MemoryAllocator* pAllocator)
{
    vmaDestroyAllocator(pAllocator->vmaAllocator);
    *pAllocator = (MemoryAllocator) {};
}