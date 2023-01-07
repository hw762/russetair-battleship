#include "clear_screen_dyn.h"

#include <flecs.h>
#include <stdlib.h>
#include <vulkan/vulkan.h>

#include "engine/vk/vk.h"
#include "vulkan/vulkan_core.h"

struct ClearScreenDynRenderer_T {
    VkClearColorValue clearColor;
};

void createClearScreenDynRenderer(const ClearScreenDynRendererCreateInfo* pCreateInfo, ClearScreenDynRenderer* pRenderer)
{
    ecs_trace("Creating [ClearScreenDynRenderer]");
    ClearScreenDynRenderer p = malloc(sizeof(struct ClearScreenDynRenderer_T));
    if (p == NULL) {
        ecs_abort(1, "Failed to create ClearScreenRenderer");
    }
    *p = (struct ClearScreenDynRenderer_T) {
        .clearColor = pCreateInfo->clearColor,
    };
    *pRenderer = p;
}

void destroyClearScreenDynRenderer(ClearScreenDynRenderer renderer)
{
    ecs_trace("Destroying [ClearScreenDynRenderer]");
    free(renderer);
}

void clearScreenDynRendererRecord(
    ClearScreenDynRenderer renderer,
    const ClearScreenDynRendererRecordInfo* pInfo)
{
    ecs_trace("Recording command buffer...");
    ecs_log_push();

    VkCommandBufferBeginInfo beginInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
    };

    VkRenderingAttachmentInfo raInfo = {
        .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
        .imageView = pInfo->view,
        .imageLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL,
        .clearValue = { .color = renderer->clearColor },
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
    };
    VkRenderingInfo rInfo = {
        .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
        .renderArea = { .extent = { .width = pInfo->width, .height = pInfo->height } },
        .layerCount = 1,
        .colorAttachmentCount = 1,
        .pColorAttachments = &raInfo,
    };
    vkCheck(vkBeginCommandBuffer(pInfo->commandBuffer, &beginInfo))
    {
        ecs_abort(1, "Failed to start command buffer");
    }
    vkCmdBeginRendering(pInfo->commandBuffer, &rInfo);
    vkCmdEndRendering(pInfo->commandBuffer);
    vkCheck(vkEndCommandBuffer(pInfo->commandBuffer))
    {
        ecs_abort(1, "Failed to end command buffer");
    }
    ecs_trace("Finished recording command buffer");
    ecs_log_pop();
}
