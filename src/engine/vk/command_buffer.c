#include "command_buffer.h"

#include <flecs.h>
#include <stdlib.h>

#include "device.h"
#include "queue.h"
#include "swapchain.h"
#include "vk.h"

CommandPool
newCommandPool(const Device* device, const Queue* queue)
{
    ecs_trace("Creating command pool");
    ecs_log_push();
    VkCommandPoolCreateInfo ci = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = queue->queueFamilyIndex
    };
    VkCommandPool commandPool;
    vkCheck(vkCreateCommandPool(device->vkDevice, &ci, NULL, &commandPool))
    {
        ecs_abort(1, "Failed to create command pool");
    }
    ecs_trace("Created VkCommandPool = %#p", commandPool);
    ecs_log_pop();
    return (CommandPool) {
        .handle = commandPool,
        .device = device,
    };
}

CommandBuffer
newCommandBuffer(const CommandPool* pool)
{
    ecs_trace("Allocating command buffer");
    ecs_log_push();

    VkCommandBufferAllocateInfo ai = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = pool->handle,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1
    };

    VkCommandBuffer commandBuffer;
    vkCheck(vkAllocateCommandBuffers(pool->device->vkDevice, &ai, &commandBuffer))
    {
        ecs_abort(1, "Failed to allocate command buffer");
    }
    ecs_trace("Allocated VkCommandBuffer = %#p", commandBuffer);

    ecs_log_pop();
    return (CommandBuffer) {
        .handle = commandBuffer,
        .commandPool = pool,
    };
}
