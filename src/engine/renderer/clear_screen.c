#include "clear_screen.h"

#include <flecs.h>
#include <stdlib.h>
#include <vulkan/vulkan.h>

#include "engine/vk/vk.h"
#include "renderer.h"

struct ClearScreenRenderer_T {
    VkClearColorValue clearColor;
};

void createClearScreenRenderer(const ClearScreenRendererCreateInfo* pCreateInfo, ClearScreenRenderer* pRenderer)
{
    ecs_trace("Creating [ClearScreenRenderer]");
    ClearScreenRenderer p = malloc(sizeof(struct ClearScreenRenderer_T));
    if (p == NULL) {
        ecs_abort(1, "Failed to create ClearScreenRenderer");
    }
    *p = (struct ClearScreenRenderer_T) {
        .clearColor = pCreateInfo->clearColor,
    };
    *pRenderer = p;
}

void destroyClearScreenRenderer(ClearScreenRenderer renderer)
{
    ecs_trace("Destroying [ClearScreenRenderer]");
    free(renderer);
}

void clearScreenRendererRecord(
    ClearScreenRenderer renderer,
    const ClearScreenRendererRecordInfo* pInfo)
{
    ecs_trace("Recording command buffer...");
    ecs_log_push();
    VkCommandBufferBeginInfo beginInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
    };

    VkClearValue clearValue = { .color = renderer->clearColor };
    VkRenderPassBeginInfo rpassBI = {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .renderPass = pInfo->renderPass,
        .renderArea = { .extent = {
                            .width = pInfo->extent.width,
                            .height = pInfo->extent.height } },
        .framebuffer = pInfo->framebuffer,
        .clearValueCount = 1,
        .pClearValues = &clearValue,
    };
    vkCheck(vkBeginCommandBuffer(pInfo->commandBuffer, &beginInfo))
    {
        ecs_abort(1, "Failed to start command buffer");
    }
    vkCmdBeginRenderPass(pInfo->commandBuffer, &rpassBI, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdEndRenderPass(pInfo->commandBuffer);
    vkCheck(vkEndCommandBuffer(pInfo->commandBuffer))
    {
        ecs_abort(1, "Failed to end command buffer");
    }
    ecs_trace("Finished recording command buffer");
    ecs_log_pop();
}
