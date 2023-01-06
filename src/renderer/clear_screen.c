#include "clear_screen.h"

#include <flecs.h>
#include <stdlib.h>
#include <vulkan/vulkan.h>

#include "vk/vk.h"

void commandBufferRecordClear(const CommandBuffer* commandBuffer,
    const Framebuffer* frameBuffer,
    const ImageView* view, uint32_t w, uint32_t h)
{
    ecs_trace("Recording command buffer...");
    ecs_log_push();
    VkCommandBufferBeginInfo beginInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
    };

    VkClearValue clearValue = { .color = { .float32 = { 0.1, 0.2, 0.3, 1.0 } } };
    VkRenderPassBeginInfo rpassBI = {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .renderPass = frameBuffer->pRenderPass->vkRenderPass,
        .renderArea = { .extent = { .width = w, .height = h } },
        .framebuffer = frameBuffer->vkFramebuffer,
        .clearValueCount = 1,
        .pClearValues = &clearValue,
    };
    vkCheck(vkBeginCommandBuffer(commandBuffer->handle, &beginInfo))
    {
        ecs_abort(1, "Failed to start command buffer");
    }
    vkCmdBeginRenderPass(commandBuffer->handle, &rpassBI, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdEndRenderPass(commandBuffer->handle);
    vkCheck(vkEndCommandBuffer(commandBuffer->handle))
    {
        ecs_abort(1, "Failed to end command buffer");
    }
    ecs_trace("Finished recording command buffer");
    ecs_log_pop();
}

void createClearScreenRenderer()
{
}
