#include "clear_screen.h"

#include <flecs.h>
#include <stdlib.h>
#include <vulkan/vulkan.h>

#include "engine/vk/vk.h"
#include "vulkan/vulkan_core.h"

struct ClearScreenRenderer_T {
    VkClearColorValue clearColor;
    VkRenderPass renderPass;
    VkFramebuffer framebuffer;
};

void createClearScreenRenderer(const ClearScreenRendererCreateInfo* pCreateInfo, ClearScreenRenderer* pRenderer)
{
    ecs_trace("Creating [ClearScreenRenderer]");
    ClearScreenRenderer p = malloc(sizeof(struct ClearScreenRenderer_T));
    if (p == NULL) {
        ecs_abort(1, "Failed to create ClearScreenRenderer");
    }

    VkAttachmentDescription colorAttachment = {
        .format = pCreateInfo->format,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
    };
    VkAttachmentReference colorReference = {
        .attachment = 0,
        .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
    };
    VkSubpassDescription subpass = {
        .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
        .colorAttachmentCount = 1,
        .pColorAttachments = &colorReference,
    };
    VkRenderPassCreateInfo renderPassCI = {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        .attachmentCount = 1,
        .pAttachments = &colorAttachment,
        .subpassCount = 1,
        .pSubpasses = &subpass,
    };
    VkRenderPass vkRenderPass;
    vkCheck(vkCreateRenderPass(pCreateInfo->device, &renderPassCI, NULL, &vkRenderPass))
    {
        ecs_abort(1, "Failed to create render pass");
    }
    VkFramebuffer framebuffer;
    VkFramebufferAttachmentImageInfo fbAII = {
        .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_ATTACHMENT_IMAGE_INFO,
        .width = pCreateInfo->width,
        .height = pCreateInfo->height,
        .layerCount = 1,
        .viewFormatCount = 1,
        .pViewFormats = &pCreateInfo->format,
        .usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
    };
    VkFramebufferAttachmentsCreateInfo fbACI = {
        .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_ATTACHMENTS_CREATE_INFO,
        .attachmentImageInfoCount = 1,
        .pAttachmentImageInfos = &fbAII,
    };
    VkFramebufferCreateInfo fbCI = {
        .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
        .renderPass = vkRenderPass,
        .flags = VK_FRAMEBUFFER_CREATE_IMAGELESS_BIT,
        .attachmentCount = 1,
        .width = pCreateInfo->width,
        .height = pCreateInfo->height,
        .pNext = &fbACI,
    };
    vkCheck(vkCreateFramebuffer(pCreateInfo->device, &fbCI, NULL, &framebuffer))
    {
        ecs_abort(1, "Failed to create [VkFrameBuffer]");
    }
    *p = (struct ClearScreenRenderer_T) {
        .clearColor = pCreateInfo->clearColor,
        .renderPass = vkRenderPass,
        .framebuffer = framebuffer,
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
    VkRenderPassAttachmentBeginInfo attachmentBI = {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_ATTACHMENT_BEGIN_INFO,
        .attachmentCount = 1,
        .pAttachments = &pInfo->view,
    };

    VkCommandBufferBeginInfo beginInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
    };

    VkClearValue clearValue = { .color = renderer->clearColor };
    VkRenderPassBeginInfo rpassBI = {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .renderPass = renderer->renderPass,
        .renderArea = { .extent = {
                            .width = pInfo->width,
                            .height = pInfo->height } },
        .framebuffer = renderer->framebuffer,
        .clearValueCount = 1,
        .pClearValues = &clearValue,
        .pNext = &attachmentBI,
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
