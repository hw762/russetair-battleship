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

void newMemoryAllocator(const VulkanInstance* pInstance, const Device* pDev, MemoryAllocator* pAllocator)
{
    VmaAllocatorCreateInfo ci = {
        .instance = pInstance->instance,
        .device = pDev->handle,
        .physicalDevice = pDev->phys->handle,
    };
    vkCheck(vmaCreateAllocator(&ci, &pAllocator->vmaAllocator))
    {
        ecs_abort(1, "Failed to create VMA allocator");
    }
}