#include "pipeline.h"

#include "device.h"
#include "swapchain.h"
#include "vk.h"

#include <flecs.h>
#include <stdlib.h>

// Pipeline
// newPipelineClearSwapchain(const Swapchain* swapchain)
// {
//     VkDevice device; // TODO
//     VkPipelineCache cache; // TODO
//     VkPipelineRenderingCreateInfoKHR prci = {
//         .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR,
//         .colorAttachmentCount = 1,
//         .pColorAttachmentFormats = swapchain->format.imageFormat
//     };
//     VkGraphicsPipelineCreateInfo ci[1] = {
//         {
//             .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
//         }
//     };
//     VkPipeline pipelines[1];
//     vkCheck(vkCreateGraphicsPipelines(device, cache, 1, ci, NULL, pipelines))
//     {
//         ecs_abort(1, "Failed to create graphics pipeline");
//     }

// }

CommandPool
newCommandPool(const RenderDevice* device)
{
    ecs_trace("Creating command pool");
    ecs_log_push();
    VkCommandPoolCreateInfo ci = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = device->queueFamilyIndex
    };
    VkCommandPool commandPool;
    vkCheck(vkCreateCommandPool(device->handle, &ci, NULL, &commandPool))
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
    vkCheck(vkAllocateCommandBuffers(pool->device->handle, &ai, &commandBuffer))
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

void commandBufferRecordClear(const CommandBuffer* commandBuffer,
    const ImageView* view, uint32_t w, uint32_t h)
{
    ecs_trace("Recording command buffer...");
    ecs_log_push();
    VkCommandBufferBeginInfo beginInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
    };

    VkRenderingAttachmentInfo colorAttachmentInfo = {
        .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
        .imageView = view->handle,
        .imageLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .clearValue = {
            .color = { .float32 = { 0.1, 0.2, 0.3, 1.0 } } }
    };

    VkRenderingInfo renderingInfo = {
        .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
        .renderArea = {
            .extent = { .width = w, .height = h } },
        .layerCount = 1,
        .colorAttachmentCount = 1,
        .pColorAttachments = &colorAttachmentInfo,
    };

    vkCheck(vkBeginCommandBuffer(commandBuffer->handle, &beginInfo))
    {
        ecs_abort(1, "Failed to begin command buffer");
    }
    vkCmdBeginRendering(commandBuffer->handle, &renderingInfo);
    vkCmdEndRendering(commandBuffer->handle);
    vkCheck(vkEndCommandBuffer(commandBuffer->handle))
    {
        ecs_abort(1, "Failed to end command buffer");
    }
    ecs_trace("Finished recording command buffer");
    ecs_log_pop();
}